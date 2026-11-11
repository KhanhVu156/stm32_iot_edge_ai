#include "bsp_lcd.h"

// Định nghĩa chân kết nối phần cứng theo sơ đồ Proteus
#define LCD_RS_PORT GPIOB
#define LCD_RS_PIN GPIO_PIN_5
#define LCD_E_PORT GPIOB
#define LCD_E_PIN GPIO_PIN_4
#define LCD_D4_PORT GPIOB
#define LCD_D4_PIN GPIO_PIN_3
#define LCD_D5_PORT GPIOB
#define LCD_D5_PIN GPIO_PIN_2
#define LCD_D6_PORT GPIOB
#define LCD_D6_PIN GPIO_PIN_1
#define LCD_D7_PORT GPIOB
#define LCD_D7_PIN GPIO_PIN_0

// Hàm nội bộ: Tạo xung Enable để LCD chốt dữ liệu
static void LCD_Enable_Pulse(void) {
    HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_SET);
    HAL_Delay(1);  // Giữ mức cao 1ms để LCD đọc dữ liệu
    HAL_GPIO_WritePin(LCD_E_PORT, LCD_E_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);  // Chờ LCD xử lý xong
}

// Hàm nội bộ: Gửi 4-bit dữ liệu (1 nibble)
static void LCD_Write_4bit(uint8_t data, uint8_t is_cmd) {
    // Thiết lập chân RS (0: lệnh, 1: dữ liệu)
    HAL_GPIO_WritePin(LCD_RS_PORT, LCD_RS_PIN,
                      is_cmd ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Đưa 4 bit dữ liệu ra các chân D4-D7
    HAL_GPIO_WritePin(LCD_D4_PORT, LCD_D4_PIN,
                      (data & 0x01) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D5_PORT, LCD_D5_PIN,
                      (data & 0x02) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D6_PORT, LCD_D6_PIN,
                      (data & 0x04) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LCD_D7_PORT, LCD_D7_PIN,
                      (data & 0x08) ? GPIO_PIN_SET : GPIO_PIN_RESET);

    // Tạo xung Enable để chốt
    LCD_Enable_Pulse();
}

// Hàm nội bộ: Gửi 8-bit dữ liệu (chia làm 2 lần gửi 4-bit)
static void LCD_Write_8bit(uint8_t data, uint8_t is_cmd) {
    // Gửi 4 bit cao (High nibble)
    LCD_Write_4bit(data >> 4, is_cmd);
    // Gửi 4 bit thấp (Low nibble)
    LCD_Write_4bit(data & 0x0F, is_cmd);
}

// Gửi lệnh đến LCD (RS = 0)
void LCD_SendCmd(uint8_t cmd) { LCD_Write_8bit(cmd, 0); }

// Gửi dữ liệu (ký tự) đến LCD (RS = 1)
void LCD_SendData(uint8_t data) { LCD_Write_8bit(data, 1); }

// Khởi tạo LCD theo đúng trình tự trong Datasheet
void LCD_Init(void) {
    HAL_Delay(50);  // Chờ nguồn ổn định (>15ms)

    // Trình tự đánh thức LCD (Wake-up sequence)
    LCD_Write_4bit(0x03, 0);
    HAL_Delay(5);
    LCD_Write_4bit(0x03, 0);
    HAL_Delay(1);
    LCD_Write_4bit(0x03, 0);
    HAL_Delay(1);

    // Chuyển sang chế độ 4-bit
    LCD_Write_4bit(0x02, 0);
    HAL_Delay(1);

    // Cấu hình: 4-bit, 2 dòng, font 5x8
    LCD_SendCmd(0x28);
    HAL_Delay(1);

    // Bật hiển thị, tắt con trỏ, tắt nhấp nháy
    LCD_SendCmd(0x0C);
    HAL_Delay(1);

    // Tự động tăng con trỏ sau khi ghi
    LCD_SendCmd(0x06);
    HAL_Delay(1);

    // Xóa màn hình
    LCD_Clear();
}

// Xóa màn hình (Lệnh 0x01)
void LCD_Clear(void) {
    LCD_SendCmd(0x01);
    HAL_Delay(2);  // Lệnh xóa cần thời gian thực thi lâu hơn (khoảng 1.52ms)
}

// Đặt vị trí con trỏ
void LCD_SetCursor(uint8_t row, uint8_t col) {
    // Edge case: Kiểm tra tọa độ hợp lệ để tránh tràn màn hình
    if (row > 1 || col > 15) {
        return;
    }

    uint8_t address;
    if (row == 0) {
        address = 0x00 + col;  // Dòng 1 bắt đầu từ 0x00
    } else {
        address = 0x40 + col;  // Dòng 2 bắt đầu từ 0x40
    }

    // Gửi lệnh Set DDRAM Address (0x80 | address)
    LCD_SendCmd(0x80 | address);
}

// Gửi 1 ký tự ra màn hình
void LCD_SendChar(char ch) { LCD_SendData((uint8_t)ch); }

// Gửi 1 chuỗi ký tự ra màn hình
void LCD_SendString(char* str) {
    // Edge case: Kiểm tra con trỏ NULL để tránh HardFault
    if (str == NULL) {
        return;
    }

    while (*str != '\0') {
        LCD_SendChar(*str);
        str++;
    }
}