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

* **Tên dự án:** STM32 Edge AI IoT Dashboard — Giám sát nhiệt độ và điều khiển thiết bị từ xa.
* **Mục tiêu:** Xây dựng hệ thống nhúng hoàn chỉnh (Bare-metal C) giao tiếp với Web thông qua cổng COM ảo, ứng dụng thuật toán phát hiện bất thường (Anomaly Detection) để cảnh báo sự cố nhiệt độ mà không cần xử lý trên Cloud.

### 2. Môi trường

* **Ngôn ngữ:** C (STM32 HAL), Python 3.9+ (Web/Gateway).
* **Thư viện sử dụng (Python):** `Flask`, `Flask-SocketIO`, `gunicorn`, `pyserial`, `python-dotenv`.
* **Phần mềm hỗ trợ:** Proteus 8+, `arm-none-eabi-gcc` (qua MSYS2/MinGW), `com0com` (tạo cổng COM ảo), Miniconda/Anaconda.

### 3. Lệnh

**Build Firmware (STM32):**

```bash
mingw32-make clean
mingw32-make -j4
```

**Chạy Web Server (Local):**

```bash
cd web_dashboard
pip install -r requirements.txt
python app.py
```

**Chạy Gateway (Kết nối Proteus với Web):**

```bash
python gateway.py
```

**Build giao diện Tailwind CSS:**

```cmd
cd web_dashboard
.\tailwindcss.exe -i ./static/css/input.css -o ./static/css/output.css --minify
```

### 4. Quy ước

* **Input (Điều khiển):** Lệnh UART dạng `LED ON x` hoặc `LED OFF x` (với x từ 1 đến 8). Lệnh `STATUS` để lấy nhiệt độ.
* **Input (Nút nhấn):** Nút START/STOP trên Proteus hoặc trên Web để thay đổi trạng thái hệ thống (RUN/STOP).
* **Format log UART:** Firmware phản hồi theo hai format cố định — `[ALERT] TEMP:<x>,ANOMALY` khi phát hiện bất thường và `[AI] TEMP:<x>,NORMAL` khi nhiệt độ bình thường. Bỏ khoảng trắng trong cặp key-value để chuỗi UART ngắn hơn, giảm nguy cơ tràn buffer khi đường truyền nhiễu.
* **Xử lý lỗi:** Bộ phân tích cú pháp UART tự động nhận diện lệnh bị dính chữ (do tràn bộ đệm) bằng hàm `strstr()`, đảm bảo không bị treo hệ thống khi có nhiễu.

### 5. Minh chứng

* **Giao diện:** Web Dashboard có Dark/Light mode, cập nhật trạng thái thời gian thực qua Socket.IO. Có nút điều khiển START/STOP từ xa. Thẻ nhiệt độ tự động đổi màu đỏ, nhấp nháy icon cảnh báo và bật **toast gradient đỏ trượt vào từ góc trên phải** mỗi khi AI phát hiện bất thường. Toast tự ẩn sau 5 giây.
* **Hiệu năng:** Tốc độ phản hồi UART < 100ms nhờ Ring Buffer và Ngắt phần cứng trực tiếp trên thanh ghi. Khi test qua Gateway (delay 20ms/byte), không mất ký tự ở 115200 baud. Khi gõ tay trên TEP Serial Monitor của Proteus, thỉnh thoảng mất một ký tự do ORE — đây là giới hạn vật lý của bộ mô phỏng, xem mục "Giới hạn".
* **Mã nguồn:** Tổ chức theo kiến trúc Module (BSP, App, Core) rõ ràng, Makefile tùy chỉnh cho Windows/Linux.

---

## 📖 Giới thiệu

**STM32 Edge AI IoT Dashboard** là một dự án Full-stack kết hợp giữa lập trình nhúng Bare-metal (C) và phát triển Web (Python/Flask). Hệ thống giải quyết bài toán truyền nhận dữ liệu qua UART ở tốc độ cao (115200 baud) mà không bị mất byte, đồng thời triển khai thuật toán **Edge AI (Z-Score thu gọn)** để giám sát nhiệt độ tại chỗ.

Board mạch đọc ADC từ LM35 theo chu kỳ 0,5 giây (TIM2, prescaler 7999, period 499 với HSI 8 MHz), lưu 5 mẫu gần nhất, và quyết định mẫu mới có bất thường hay không dựa trên độ lệch so với trung bình động.

Dữ liệu UART đi theo một trong hai đường. Ở chế độ local, `app.py` tự mở COM trực tiếp và đọc log. Ở chế độ production, `gateway.py` mở COM, forward từng dòng lên Render qua Socket.IO, rồi Render emit xuống trình duyệt. Cả hai đường đều dùng chung một event `serial_data` nên frontend không cần phân biệt nguồn.

Điểm nhấn kỹ thuật là cách xử lý UART. Firmware đọc trực tiếp thanh ghi `USART1->SR` và `USART1->DR` thay vì gọi `HAL_UART_Receive_IT`, nhờ vậy kiểm soát được cờ ORE — thứ mà HAL xử lý khá mập mờ khi RXNE đang set. Chi tiết trong mục "Cập nhật gần đây".

---

## 🚀 Tính năng nổi bật

### 1. Kiến trúc Phân tán & Tối ưu hóa (System Architecture)

* **Ring Buffer UART (C):** Hứng dữ liệu bằng ngắt trực tiếp trên thanh ghi, chống tràn bộ đệm (Overrun Error) và mất ký tự ở tốc độ 115200. Khi `next_head == rx_tail`, byte mới bị drop thay vì ghi đè — chấp nhận mất byte thay vì corrupt dữ liệu đang chờ xử lý.
* **Edge AI (Anomaly Detection):** Tự động phát hiện nhiệt độ tăng/giảm đột ngột (>2°C) so với trung bình 5 mẫu gần nhất. Thuật toán gọn đủ để chạy trên Cortex-M3 ở 8 MHz mà không ảnh hưởng chu kỳ lấy mẫu.
* **Hai kênh điều khiển độc lập:** 8 LED trên Port B (PB8–PB15) chỉ phản ứng với lệnh `LED ON/OFF` từ UART. Thuật toán AI tuyệt đối không chạm vào 8 LED này — nếu AI muốn cảnh báo, nó in log và cập nhật LCD, phần hiển thị trực quan do web đảm nhận.
* **Gateway Python (pyserial + SocketIO):** Cầu nối không dây giữa phần cứng (Proteus) và Cloud (Render). Gửi từng byte với delay 20ms thay vì `write()` cả chuỗi, vì UART ảo của Proteus xử lý burst rất kém.

### 2. Trực quan hóa & Trải nghiệm UI/UX

* **Hệ thống Template Kép (Flask):** Tách biệt cấu trúc giao diện, sử dụng `url_for` để quản lý tài nguyên tĩnh (CSS/JS) chuyên nghiệp.
* **Dark/Light Mode:** Chuyển đổi giao diện mượt mà, lưu trạng thái vào `localStorage` của trình duyệt, toggle không reload trang.
* **Custom Toggle Switch:** Thiết kế nút gạt iOS-style để bật/tắt 8 thiết bị LED trực quan, kèm cơ chế khóa 300ms sau mỗi lần bấm để tránh user spam click làm loạn trạng thái checkbox.
* **Realtime Warning:** Thẻ nhiệt độ chuyển sang màu đỏ, nhấp nháy và hiện icon Tam giác cảnh báo khi AI phát hiện bất thường.
* **Toast cảnh báo AI:** Gradient đỏ, trượt vào từ phải trong 350ms, tự ẩn sau 5 giây. Khi anomaly kéo dài, mỗi log mới refresh timer thay vì stack nhiều toast. User bấm nút X thì snooze 10 giây — tránh toast nhấp nháy liên tục trong lúc chờ nhiệt độ hạ.

---

## 🆕 Cập nhật gần đây

### Fix mất ký tự UART do xử lý ORE sai

Trước đây, mỗi lần gõ nhanh trên TEP Serial Monitor, chuỗi `LED ON 01` biến thành `LED O 1` rồi parser báo "lệnh không hợp lệ". Nguyên nhân nằm trong `USART1_IRQHandler`: khi cùng lúc có cờ ORE và RXNE, code cũ đọc `USART1->DR` hai lần.

Theo RM0008 mục 27.6.1, đọc `SR` rồi đọc `DR` một lần là đủ clear cả hai cờ, và byte trả về là byte hợp lệ cuối cùng trong DR. Code cũ đọc DR lần đầu để clear ORE — tức là vứt luôn byte hợp lệ — rồi đọc DR lần hai khi RXNE vẫn còn set, nhận về 0x00 và push vào Ring Buffer. Mỗi lần tràn, hệ thống mất hai ký tự thay vì một.

Firmware mới snapshot `SR` một lần, rồi đọc `DR` đúng một lần nếu có bất kỳ cờ nào trong `(USART_SR_RXNE | USART_SR_ORE)`. Kết quả: mỗi lần ORE chỉ mất byte gây tràn (byte vật lý không thể khôi phục), byte hợp lệ trước đó vẫn được đẩy vào buffer.

### Tách 8 LED khỏi logic AI

Trước đây, mỗi lần TIM2 tràn (0,5 giây), nhánh `else` của thuật toán AI (nhiệt độ bình thường) chạy vòng `for` tắt cả 8 LED. Hậu quả: bấm `LED ON 1` trên web, đèn sáng đúng 0,5 giây rồi tắt ngóm. Người dùng nhìn vào tưởng lệnh không có tác dụng, dù log vẫn báo "Da BAT LED 1".

Firmware mới bỏ cả hai vòng lặp `System_SetLedState` khỏi nhánh AI. 8 LED giờ chỉ thay đổi trạng thái khi có lệnh `LED ON/OFF` từ UART — trạng thái được giữ vĩnh viễn cho đến khi có lệnh ngược lại. Cảnh báo AI chuyển hoàn toàn sang kênh hiển thị: log UART, LCD và toast trên web.

### Chuẩn hóa format log

Firmware cũ in `[AI] TEMP: 30 C, AI: NORMAL`. Format mới là `[AI] TEMP:30,NORMAL`, và dùng tiền tố `[ALERT]` thay `[AI]` khi phát hiện bất thường: `[ALERT] TEMP:35,ANOMALY`. Bỏ khoảng trắng trong cặp key-value để parser không phải trim, đồng thời tách biệt hai loại sự kiện ngay từ dòng đầu tiên.

Parser JavaScript nhận cả hai format cũ và mới để không phá vỡ luồng log của firmware chưa flash lại. Trigger RUNNING cũng được mở rộng để bao gồm `[ALERT]` — tránh trường hợp hệ thống chỉ phát alert mà card trạng thái vẫn hiển thị STOPPED.

### Toast cảnh báo AI 5 giây

Trình duyệt bắt dòng `[ALERT] TEMP:<x>,ANOMALY`, parse nhiệt độ, và gọi `showAnomalyToast()`. Toast hiện ở góc trên phải, dưới header, tự ẩn sau 5 giây. Khi anomaly tiếp diễn, mỗi alert mới reset timer thay vì đẩy thêm toast vào stack.

Nút X trên toast ẩn ngay lập tức và đặt snooze 10 giây. Nếu anomaly kéo dài vài phút, toast không nhấp nháy liên tục làm phiền người dùng. Khi có dòng `[AI] ...NORMAL`, toast ẩn ngay bất kể trạng thái snooze.

Bỏ trường hợp `TEMP: ERR` (cảm biến lỗi) khỏi việc hiện toast — chỉ cập nhật log và thẻ nhiệt độ, vì đây là lỗi đọc ADC chứ không phải bất thường nhiệt độ.

---

## 🛠️ Công nghệ sử dụng

| Thành phần | Công nghệ |
| --- | --- |
| **Firmware** | C, STM32 HAL, ARM Cortex-M3, bare-metal |
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
│   │       └── stm32f1xx_it.c# Ngắt UART tự viết (chống tràn ORE)
│   ├── App/
│   │   ├── Inc/
│   │   └── Src/
│   │       ├── app_sensor.c  # Thuật toán AI (Z-score)
│   │       └── app_uart.c    # Ring Buffer, xử lý lệnh START/STOP
│   ├── Bsp/
│   └── Makefile
│
├── schematics/               # File mô phỏng Proteus
│   └── stm32f103c8_project_proteus.pdsprj
│
└── web_dashboard/            # Web Server & Gateway
    ├── app.py                # Flask Server (Render + Local)
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

Khuyến nghị dùng Conda thay vì `virtualenv` thuần để tránh xung đột thư viện C với hệ thống. Tạo môi trường Python 3.9 rồi kích hoạt:

```bash
conda create -n stm32_env python=3.9 -y
conda activate stm32_env
```

### Bước 2: Cài đặt thư viện Python

Di chuyển vào `web_dashboard` và cài các gói phụ thuộc:

```bash
cd web_dashboard
pip install -r requirements.txt
```

### Bước 3: Build giao diện Tailwind CSS

Dự án không dùng CDN cho CSS nội bộ. Mỗi lần sửa `index.html` hoặc `input.css`, phải build lại `output.css`:

```cmd
cd web_dashboard
.\tailwindcss.exe -i ./static/css/input.css -o ./static/css/output.css --minify
```

### Bước 4: Cấu hình cổng COM ảo (Proteus & Gateway)

Dùng `com0com` để tạo cặp cổng COM, ví dụ COM2 và COM3.

* Cổng **COM2** cấu hình trong Proteus (TEP Serial Monitor).
* Cổng **COM3** cấu hình trong file `.env` (biến `COM_PORT`). Gateway và Flask đọc chung biến này, nên không cần khai báo hai lần.

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

*(Lưu ý: Mở file `.env` để cập nhật `RENDER_SERVER_URL` trỏ tới link Render thực tế. Nếu chỉ test local, để nguyên `http://127.0.0.1:5000`.)*

---

## ⚠️ Giới hạn & Lưu ý kỹ thuật

* **Lỗi xung đột Clock trên Proteus:** Code C sử dụng **Dao động nội HSI (8MHz)** thay vì thạch anh ngoài để tránh lỗi mô phỏng thời gian thực trên Proteus. Hệ quả là baud rate UART có sai số nhỏ so với tính toán trên giấy — chấp nhận được ở 115200 vì sai số dưới 2%.
* **Render Free Tier (512MB RAM):** Render sẽ tự động "ngủ đông" nếu không có truy cập trong 15 phút. Sử dụng **UptimeRobot** để ping mỗi 5 phút giữ web luôn "thức".
* **Tốc độ UART 115200 trên Proteus:** Proteus xử lý UART ảo rất chậm. Script Gateway (`gateway.py`) đã được cấu hình gửi từng byte với delay 20ms để tránh mất ký tự. Nhưng khi gõ tay trực tiếp trên TEP Monitor, các ký tự đến theo burst và có thể gây ORE — mỗi lần tràn mất đúng một ký tự (byte vật lý). Đây là giới hạn vật lý của bộ mô phỏng, không phải bug firmware. Muốn tránh, gõ chậm khoảng 200ms/ký tự.
* **Đồng bộ CSS (Tailwind CLI):** Bất kỳ class Tailwind mới nào được thêm vào `index.html` đều yêu cầu phải chạy lại lệnh build CSS (Bước 3) thì giao diện mới cập nhật.
* **Toast cảnh báo AI:** Chỉ hoạt động khi tab trình duyệt đang mở. Nếu đóng tab lúc anomaly xảy ra, lịch sử cảnh báo không được lưu lại — muốn xem lại phải đọc log UART trong terminal.

---

## 🚢 Triển khai Online (Live Demo)

👉 **[Live Demo trên Render](https://stm32-iot-edge-ai.onrender.com/)**

---

## 📝 License

Dự án phục vụ mục đích học tập và phát triển cá nhân.
