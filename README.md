# 🎯 STM32 Edge AI IoT Dashboard

> **Đồ án Hệ thống Nhúng & IoT**  
> Hệ thống giám sát và điều khiển từ xa sử dụng vi điều khiển **STM32F103C8T6**, kết hợp cảm biến nhiệt độ **LM35**, mô phỏng trên **Proteus** và giao diện Web Dashboard thời gian thực. Dự án tích hợp **Edge AI** (Trí tuệ nhân tạo tại biên) để tự động phát hiện bất thường nhiệt độ ngay trên vi điều khiển.

[![Live Demo](https://img.shields.io/badge/Live%20Demo-Render-brightgreen)](https://stm32-iot-edge-ai.onrender.com/)
[![C Language](https://img.shields.io/badge/C-Embedded-blue)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Python](https://img.shields.io/badge/Python-3.9+-yellow)](https://www.python.org/)
[![Flask](https://img.shields.io/badge/Flask-Web%20Framework-lightgrey)](https://flask.palletsprojects.com/)
[![STM32](https://img.shields.io/badge/STM32-ARM%20Cortex--M3-darkblue)](https://www.st.com/)
[![Tailwind](https://img.shields.io/badge/Tailwind-CSS-38B2AC)](https://tailwindcss.com/)
[![Proteus](https://img.shields.io/badge/Proteus-Simulation-red)](https://www.labcenter.com/)

---

## 📑 THÔNG TIN DỰ ÁN & HƯỚNG DẪN ĐỐI CHIẾU

### 1. Nhận diện

* **Tên dự án:** STM32 Edge AI IoT Dashboard - Giám sát nhiệt độ và điều khiển thiết bị từ xa.
* **Mục tiêu:** Xây dựng hệ thống nhúng hoàn chỉnh (Bare-metal C) giao tiếp với Web thông qua cổng COM ảo, ứng dụng thuật toán phát hiện bất thường (Anomaly Detection) để cảnh báo sự cố nhiệt độ mà không cần xử lý trên Cloud.

### 2. Môi trường

* **Ngôn ngữ:** C (STM32 HAL), Python 3.9+ (Web/Gateway).
* **Thư viện sử dụng (Python):** `Flask`, `Flask-SocketIO`, `gunicorn`, `pyserial`, `python-dotenv`.
* **Phần mềm hỗ trợ:** Proteus 8+, `arm-none-eabi-gcc` (qua MSYS2/MinGW), `com0com` (tạo cổng COM ảo), Miniconda/Anaconda.

### 3. Lệnh

* **Build Firmware (STM32):**
  `mingw32-make clean`
  `mingw32-make -j4`
* **Chạy Web Server (Local):**
  `pip install -r requirements.txt`
  `python app.py`
* **Chạy Gateway (Kết nối Proteus với Web):**
  `python gateway.py`
* **Build giao diện Tailwind CSS:**
  `.\tailwindcss.exe -i ./static/css/input.css -o ./static/css/output.css --minify`

### 4. Quy ước

* **Input (Điều khiển):** Lệnh UART dạng `LED ON x` hoặc `LED OFF x` (với x từ 1 đến 8). Lệnh `STATUS` để lấy nhiệt độ.
* **Input (Nút nhấn):** Nút START/STOP trên Proteus hoặc trên Web để thay đổi trạng thái hệ thống (RUN/STOP).
* **Xử lý lỗi:** Bộ phân tích cú pháp UART tự động nhận diện lệnh bị dính chữ (do tràn bộ đệm) bằng hàm `strstr()`, đảm bảo không bị treo hệ thống khi có nhiễu.

### 5. Minh chứng

* **Giao diện:** Web Dashboard có Dark/Light mode, cập nhật trạng thái thời gian thực qua Socket.IO. Có nút điều khiển START/STOP từ xa. Thẻ nhiệt độ tự động đổi màu đỏ và hiện icon cảnh báo khi AI phát hiện bất thường.
* **Hiệu năng:** Tốc độ phản hồi UART < 100ms nhờ Ring Buffer và Ngắt phần cứng trực tiếp.
* **Mã nguồn:** Tổ chức theo kiến trúc Module (BSP, App, Core) rõ ràng, Makefile tùy chỉnh cho Windows/Linux.

---

## 📖 Giới thiệu

**STM32 Edge AI IoT Dashboard** là một dự án Full-stack kết hợp giữa lập trình nhúng Bare-metal (C) và phát triển Web (Python/Flask). Hệ thống giải quyết bài toán truyền nhận dữ liệu qua UART ở tốc độ cao (115200 baud) mà không bị mất byte, đồng thời triển khai thuật toán **Edge AI (Z-Score thu gọn)** để giám sát nhiệt độ tại chỗ.

Điểm nhấn của dự án là **Gateway Python** - một cầu nối giúp đưa dữ liệu từ mạch mô phỏng Proteus (chạy trên máy tính) lên Cloud (Render) để có thể giám sát và điều khiển từ bất kỳ đâu có Internet.

---

## 🚀 Tính năng nổi bật

### 1. Kiến trúc Phân tán & Tối ưu hóa (System Architecture)

* **Ring Buffer UART (C):** Hứng dữ liệu bằng ngắt trực tiếp trên thanh ghi, chống tràn bộ đệm (Overrun Error) và mất ký tự ở tốc độ 115200.
* **Edge AI (Anomaly Detection):** Tự động phát hiện nhiệt độ tăng/giảm đột ngột (>2°C) so với trung bình 5 mẫu gần nhất.
* **Cơ chế Latch:** Khi có bất thường, hệ thống sẽ duy trì cảnh báo trong 3 giây (bật cả 8 LED) để người dùng dễ nhận biết.
* **Gateway Python (pyserial + SocketIO):** Cầu nối không dây giữa phần cứng (Proteus) và Cloud (Render).

### 2. Trực quan hóa & Trải nghiệm UI/UX

* **Hệ thống Template Kép (Flask):** Tách biệt cấu trúc giao diện, sử dụng `url_for` để quản lý tài nguyên tĩnh (CSS/JS) chuyên nghiệp.
* **Dark/Light Mode:** Chuyển đổi giao diện mượt mà, lưu trạng thái vào `localStorage` của trình duyệt.
* **Custom Toggle Switch:** Thiết kế nút gạt iOS-style để bật/tắt 8 thiết bị LED trực quan.
* **Realtime Warning:** Thẻ nhiệt độ chuyển sang màu đỏ, nhấp nháy và hiện icon Tam giác cảnh báo khi AI phát hiện bất thường.

---

## 🛠️ Công nghệ sử dụng

| Thành phần | Công nghệ |
| ------------ | ----------- |
| **Firmware** | C, STM32 HAL, ARM Cortex-M3, Bare-metal |
| **Mô phỏng** | Proteus, `com0com` (Virtual Serial Port) |
| **Backend** | Python 3.9+, Flask, Flask-SocketIO |
| **Frontend** | Tailwind CSS (CLI), Vanilla JS, Socket.IO Client |
| **Gateway** | Python, pyserial |
| **Deployment** | Gunicorn (gthread), Render |

---

## 📂 Cấu trúc thư mục

```text
stm32-edge-ai-iot/
├── firmware/                 # Code C cho STM32
│   ├── Core/
│   │   ├── Inc/
│   │   └── Src/
│   │       ├── main.c        # Logic chính, quản lý trạng thái
│   │       └── stm32f1xx_it.c# Ngắt UART tự viết (chống tràn)
│   ├── App/
│   │   ├── Inc/
│   │   └── Src/
│   │       ├── app_sensor.c  # Thuật toán AI (Latch, Z-score)
│   │       └── app_uart.c    # Ring Buffer, xử lý lệnh START/STOP
│   ├── Bsp/
│   └── Makefile
│
├── schematics/               # File mô phỏng Proteus
│   └── stm32f103c8_project_proteus.pdsprj
│
└── web_dashboard/            # Web Server & Gateway
    ├── app.py                # Flask Server (Render)
    ├── gateway.py            # Script đọc COM & gửi lên Render
    ├── requirements.txt
    ├── Procfile
    ├── templates/
    │   └── index.html        # Giao diện Dashboard
    └── static/
        └── css/
            ├── input.css
            └── output.css
```

---

## ⚙️ Hướng dẫn cài đặt & Chạy

### Bước 1: Khởi tạo môi trường Conda

Khuyến nghị sử dụng Conda để quản lý môi trường ảo, tránh xung đột thư viện với hệ thống.

```bash
# Tạo môi trường mới với Python 3.9
conda create -n stm32_env python=3.9 -y

# Kích hoạt môi trường
conda activate stm32_env
```

### Bước 2: Cài đặt thư viện

Di chuyển vào thư mục `web_dashboard` và cài đặt các gói phụ thuộc.

```bash
cd web_dashboard
pip install -r requirements.txt
```

### Bước 3: Build giao diện Tailwind CSS

Do dự án không sử dụng CDN cho các trang nội bộ, cần build file CSS tĩnh từ Tailwind CLI.

```bash
.\tailwindcss.exe -i ./static/css/input.css -o ./static/css/output.css --minify
```

### Bước 4: Cấu hình Cổng COM ảo (Proteus & Gateway)

Sử dụng phần mềm `com0com` để tạo cặp cổng COM (ví dụ: COM2 và COM3).

* Cổng **COM2** cấu hình trong Proteus (TEP Serial Monitor).
* Cổng **COM3** cấu hình trong file `.env` (biến `COM_PORT`).

### Bước 5: Khởi chạy Web Server (Local)

```bash
python app.py
```

Truy cập giao diện tại: **<http://127.0.0.1:5000>**

### Bước 6: Khởi chạy Gateway (Kết nối Proteus với Cloud)

Mở một cửa sổ CMD mới (đã kích hoạt `stm32_env`), chạy lệnh:

```bash
python gateway.py
```

*(Lưu ý: Mở file `.env` để cập nhật `RENDER_SERVER_URL` trỏ tới link Render thực tế).*

---

## ⚠️ Giới hạn & Lưu ý kỹ thuật

* **Lỗi xung đột Clock trên Proteus:** Code C sử dụng **Dao động nội HSI (8MHz)** thay vì thạch anh ngoài để tránh lỗi mô phỏng thời gian thực trên Proteus.
* **Render Free Tier (512MB RAM):** Render sẽ tự động "ngủ đông" nếu không có truy cập trong 15 phút. Sử dụng **UptimeRobot** để ping giữ web luôn "thức".
* **Tốc độ UART 115200 trên Proteus:** Proteus xử lý UART ảo rất chậm. Script Gateway (`gateway.py`) đã được cấu hình gửi từng byte với delay 20ms để tránh mất ký tự.
* **Đồng bộ CSS (Tailwind CLI):** Bất kỳ class Tailwind mới nào được thêm vào `index.html` đều yêu cầu phải chạy lại lệnh build CSS (Bước 3) thì giao diện mới cập nhật.

---

## 🚢 Triển khai Online (Live Demo)

👉 **[Live Demo trên Render](https://stm32-iot-edge-ai.onrender.com/)**

---

## 📝 License

Dự án phục vụ mục đích học tập và phát triển cá nhân.
