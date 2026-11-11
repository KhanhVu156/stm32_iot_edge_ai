#ifndef __BSP_LCD_H
#define __BSP_LCD_H

#include "main.h"  // Cần thiết để dùng các kiểu dữ liệu và hàm HAL

// Khởi tạo màn hình LCD
void LCD_Init(void);

// Xóa toàn bộ màn hình
void LCD_Clear(void);

// Đặt vị trí con trỏ (row: 0-1, col: 0-15)
void LCD_SetCursor(uint8_t row, uint8_t col);

// Gửi 1 ký tự ra màn hình
void LCD_SendChar(char ch);

// Gửi 1 chuỗi ký tự ra màn hình
void LCD_SendString(char* str);

#endif /* __BSP_LCD_H */