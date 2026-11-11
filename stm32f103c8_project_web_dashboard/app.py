import serial
import time
import threading
import os
from dotenv import load_dotenv
from flask import Flask, render_template, request
from flask_socketio import SocketIO

load_dotenv() # Nạp biến môi trường từ .env

app = Flask(__name__)
app.config['SECRET_KEY'] = os.environ.get('SECRET_KEY', 'secret!')
# Sử dụng threading thay vì eventlet để tránh lỗi C++ trên Windows
socketio = SocketIO(app, cors_allowed_origins="*", async_mode='threading')

# Cấu hình COM (Chỉ dùng khi chạy Local)
COM_PORT = os.environ.get('COM_PORT', 'COM3')
BAUD_RATE = int(os.environ.get('BAUD_RATE', 115200))
ser = None
serial_lock = threading.Lock()

def init_serial():
    global ser
    try:
        ser = serial.Serial(COM_PORT, BAUD_RATE, timeout=1)
        print(f"[PYTHON] Da ket noi {COM_PORT} thanh cong.")
    except Exception as e:
        print(f"[PYTHON][ERROR] Khong the mo {COM_PORT}: {e}")
        print("[PYTHON] He thong se cho Gateway ket noi...")

def read_from_com():
    global ser
    while True:
        if ser and ser.is_open:
            try:
                with serial_lock:
                    if ser.in_waiting > 0:
                        line = ser.readline().decode('utf-8', errors='ignore').strip()
                        if line:
                            print(f"[STM32] {line}")
                            socketio.emit('serial_data', {'data': line})
            except Exception as e:
                print(f"[PYTHON][ERROR] Loi doc COM: {e}")
                time.sleep(1)
        time.sleep(0.05)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/send_command', methods=['POST'])
def send_command():
    global ser
    cmd = request.json.get('command')
    if not cmd:
        return {'status': 'error', 'message': 'Lệnh rỗng'}
    
    print(f"[WEB] Nhan lenh tu giao dien: {cmd}")
    
    # 1. Luôn gửi lệnh qua Socket.IO để Gateway (nếu có) nhận được
    socketio.emit('gateway_command', {'command': cmd})
    
    # 2. Nếu đang chạy Local và có cổng COM, gửi trực tiếp
    if ser and ser.is_open:
        try:
            with serial_lock:
                for char in cmd:
                    ser.write(char.encode('utf-8'))
                    ser.flush()
                    time.sleep(0.02) 
                time.sleep(0.05)
                ser.write(b'\r\n')
                ser.flush()
                time.sleep(0.05)
            return {'status': 'success', 'message': 'Đã gửi trực tiếp qua COM'}
        except Exception as e:
            return {'status': 'error', 'message': str(e)}
            
    return {'status': 'success', 'message': 'Đã gửi lệnh qua Gateway'}

# === CÁC SỰ KIỆN SOCKET.IO CHO GATEWAY ===
@socketio.on('gateway_connect')
def handle_gateway_connect():
    print("[RENDER] Gateway (May tinh ca nhan) da ket noi thanh cong!")

@socketio.on('gateway_data')
def handle_gateway_data(data):
    socketio.emit('serial_data', {'data': data})

if __name__ == '__main__':
    if os.environ.get('LOCAL_MODE', '1') == '1':
        init_serial()
        com_thread = threading.Thread(target=read_from_com, daemon=True)
        com_thread.start()
    
    port = int(os.environ.get('PORT', 5000))
    print(f"[PYTHON] Web Server dang chay tai http://0.0.0.0:{port}")
    socketio.run(app, host='0.0.0.0', port=port, allow_unsafe_werkzeug=True)