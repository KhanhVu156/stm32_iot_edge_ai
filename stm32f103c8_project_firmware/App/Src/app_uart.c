#include "app_uart.h"

#include <stdio.h>
#include <string.h>

#include "app_sensor.h"

// Khai báo extern biến timer từ main.c
extern TIM_HandleTypeDef htim2;

#define RX_BUFFER_SIZE 128
volatile uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint16_t rx_head = 0;
volatile uint16_t rx_tail = 0;

char uart_buffer[UART_BUFFER_SIZE];
uint16_t uart_idx = 0;
volatile uint8_t flag_new_command = 0;

void UART_Init(void) {
    uart_idx = 0;
    memset(uart_buffer, 0, sizeof(uart_buffer));
    rx_head = 0;
    rx_tail = 0;

    // Cấu hình ngắt trực tiếp trên thanh ghi, không dùng HAL_UART_Receive_IT
    USART1->CR1 |= USART_CR1_RXNEIE;  // Bật ngắt nhận dữ liệu

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

    printf("\r\n[UART] Khoi tao UART1 thanh cong. San sang nhan lenh.\r\n");
}

// Hàm này được gọi từ ngắt USART1_IRQHandler
void UART_PushByte(uint8_t data) {
    uint16_t next_head = (rx_head + 1) % RX_BUFFER_SIZE;
    if (next_head != rx_tail) {
        rx_buffer[rx_head] = data;
        rx_head = next_head;
    }
}

// Hàm lấy dữ liệu từ Ring Buffer ra xử lý
uint8_t UART_GetByte(uint8_t* data) {
    if (rx_head == rx_tail) return 0;
    *data = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % RX_BUFFER_SIZE;
    return 1;
}

void UART_ProcessByte(uint8_t rx_byte) {
    if (rx_byte == '\r' || rx_byte == '\n') {
        if (uart_idx > 0) {
            uart_buffer[uart_idx] = '\0';
            flag_new_command = 1;
            uart_idx = 0;
        }
    } else {
        if (uart_idx < UART_BUFFER_SIZE - 1) {
            uart_buffer[uart_idx++] = rx_byte;
        }
    }
}

void UART_ProcessCommand(void) {
    // Xử lý lệnh START/STOP (Không phụ thuộc vào sys_state)
    if (strcmp(uart_buffer, "START") == 0) {
        if (sys_state == 0) {
            sys_state = 1;
            HAL_TIM_Base_Start_IT(&htim2);
            printf("[SYSTEM] Da nhan START (tu Web) -> Chuyen sang RUN\r\n");
        }
        return;
    }
    if (strcmp(uart_buffer, "STOP") == 0) {
        if (sys_state == 1) {
            sys_state = 0;
            HAL_TIM_Base_Stop_IT(&htim2);
            printf("[SYSTEM] Da nhan STOP (tu Web) -> Chuyen sang STOP\r\n");
        }
        return;
    }

    if (sys_state == 0) {
        printf("[UART] He thong dang STOP. Bo qua lenh.\r\n");
        return;
    }

    if (strstr(uart_buffer, "STATUS") != NULL) {
        if (last_temp == TEMP_ERROR) {
            printf("[UART] Phan hoi: STATE: RUN; TEMP: N/A\r\n");
        } else {
            printf("[UART] Phan hoi: STATE: RUN; TEMP: %d C\r\n", last_temp);
        }
        return;
    }

    char* led_ptr = strstr(uart_buffer, "LED");
    if (led_ptr != NULL) {
        int led_num = 0;
        for (int i = 0; i < strlen(led_ptr); i++) {
            if (led_ptr[i] >= '1' && led_ptr[i] <= '8') {
                led_num = led_ptr[i] - '0';
                break;
            }
        }

        if (led_num >= 1 && led_num <= 8) {
            if (strstr(led_ptr, "OFF") != NULL) {
                System_SetLedState(led_num, 0);
                printf("[UART] Da TAT LED %d\r\n", led_num);
                return;
            } else if (strstr(led_ptr, "ON") != NULL) {
                System_SetLedState(led_num, 1);
                printf("[UART] Da BAT LED %d\r\n", led_num);
                return;
            }
        }
    }
    printf("[UART][ERROR] Lenh khong hop le: '%s'\r\n", uart_buffer);
}