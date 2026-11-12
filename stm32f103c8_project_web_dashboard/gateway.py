import serial
import time
import socketio
import threading
import os
from dotenv import load_dotenv

load_dotenv()

COM_PORT = os.environ.get('COM_PORT', 'COM3')
BAUD_RATE = int(os.environ.get('BAUD_RATE', 115200))
RENDER_SERVER_URL = os.environ.get('RENDER_SERVER_URL', 'http://127.0.0.1:5000')

sio = socketio.Client()

# Biến toàn cục và lock cho cổng COM
ser = None
serial_lock = threading.Lock()

def init_serial():
    global ser
    try:
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
        print(f"[GATEWAY] Da mo cong {COM_PORT} thanh cong.")
    except Exception as e:
        print(f"[GATEWAY][ERROR] Khong the mo cong {COM_PORT}: {e}")
        print("Hay chac chan rang Proteus dang chay va cong COM da duoc tao.")

@sio.event
def connect():
    print(f"[GATEWAY] Da ket noi toi server: {RENDER_SERVER_URL}")
    sio.emit('gateway_connect')

@sio.event
def disconnect():
    print("[GATEWAY] Mat ket noi toi server. Dang thu ket noi lai...")

@sio.on('gateway_command')
def on_gateway_command(data):
    global ser
    command = data.get('command')
    if command and ser and ser.is_open:
        print(f"[GATEWAY] Nhan lenh tu Server: '{command}'")
        try:
            with serial_lock:
                for char in command:
                    ser.write(char.encode('utf-8'))
                    ser.flush()
                    time.sleep(0.02)
                time.sleep(0.05)
                ser.write(b'\r\n')
                ser.flush()
                time.sleep(0.05)
        except Exception as e:
            print(f"[GATEWAY][ERROR] Khong the gui lenh xuong COM: {e}")

def read_from_com_and_forward():
    global ser
    while True:
        if ser and ser.is_open:
            try:
                with serial_lock:
                    if ser.in_waiting > 0:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        if line:
                            sio.emit('gateway_data', line)
            except Exception as e:
                print(f"[GATEWAY][ERROR] Loi doc COM: {e}")
                time.sleep(1)
        time.sleep(0.05)

if __name__ == '__main__':
    init_serial()
    
    com_thread = threading.Thread(target=read_from_com_and_forward, daemon=True)
    com_thread.start()

    while True:
        try:
            print(f"[GATEWAY] Dang ket noi toi {RENDER_SERVER_URL}...")
            sio.connect(RENDER_SERVER_URL, wait_timeout=120)
            sio.wait()
        except socketio.exceptions.ConnectionError as e:
            print(f"[GATEWAY][ERROR] Khong the ket noi toi server Render: {e}")
            print("[GATEWAY] Thu ket noi lai sau 10 giay...")
            time.sleep(10)
        except Exception as e:
            print(f"[GATEWAY][ERROR] Loi khong xac dinh: {e}")
            time.sleep(10)