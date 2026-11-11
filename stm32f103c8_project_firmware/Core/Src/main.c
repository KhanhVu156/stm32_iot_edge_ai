/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 */
/* USER CODE END Header */

#include "main.h"

#include <stdio.h>
#include <string.h>

#include "adc.h"
#include "app_sensor.h"
#include "app_uart.h"
#include "bsp_lcd.h"
#include "gpio.h"
#include "tim.h"
#include "usart.h"

void SystemClock_Config(void);
void Error_Handler(void);

#ifdef __GNUC__
int __io_putchar(int ch) {
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 0xFFFF);
    return ch;
}
#endif

volatile uint8_t sys_state = 0;
volatile uint8_t led_run_state = 0;
int last_temp = TEMP_ERROR;

volatile uint8_t flag_update_lcd_state = 0;
volatile uint8_t flag_read_temp = 0;

extern volatile uint8_t flag_new_command;
extern char uart_buffer[UART_BUFFER_SIZE];

void System_SetLedState(uint8_t led_num, uint8_t state) {
    uint16_t pin = 0;
    switch (led_num) {
        case 1:
            pin = GPIO_PIN_8;
            break;
        case 2:
            pin = GPIO_PIN_9;
            break;
        case 3:
            pin = GPIO_PIN_10;
            break;
        case 4:
            pin = GPIO_PIN_11;
            break;
        case 5:
            pin = GPIO_PIN_12;
            break;
        case 6:
            pin = GPIO_PIN_13;
            break;
        case 7:
            pin = GPIO_PIN_14;
            break;
        case 8:
            pin = GPIO_PIN_15;
            break;
        default:
            return;
    }
    HAL_GPIO_WritePin(GPIOB, pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    static uint32_t last_press_time_start = 0;
    static uint32_t last_press_time_stop = 0;
    uint32_t current_time = HAL_GetTick();

    if (GPIO_Pin == GPIO_PIN_0) {
        if ((current_time - last_press_time_start) > 200) {
            last_press_time_start = current_time;
            if (sys_state == 0) {
                sys_state = 1;
                flag_update_lcd_state = 1;
                HAL_TIM_Base_Start_IT(&htim2);
                printf("[SYSTEM] Da nhan START -> Chuyen sang RUN\r\n");
            }
        }
    } else if (GPIO_Pin == GPIO_PIN_1) {
        if ((current_time - last_press_time_stop) > 200) {
            last_press_time_stop = current_time;
            if (sys_state == 1) {
                sys_state = 0;
                flag_update_lcd_state = 1;
                HAL_TIM_Base_Stop_IT(&htim2);
                printf("[SYSTEM] Da nhan STOP -> Chuyen sang STOP\r\n");
            }
        }
    }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim) {
    if (htim->Instance == TIM2) {
        if (sys_state == 1) {
            led_run_state = !led_run_state;
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,
                              led_run_state ? GPIO_PIN_SET : GPIO_PIN_RESET);
            flag_read_temp = 1;
        }
    }
}

int main(void) {
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_ADC1_Init();
    MX_TIM2_Init();
    MX_USART1_UART_Init();

    htim2.Init.Prescaler = 7999;
    htim2.Init.Period = 499;
    HAL_TIM_Base_Init(&htim2);

    Sensor_Init();
    LCD_Init();
    UART_Init();

    printf("\r\n=== HE THONG BAT DAU (HSI 8MHz) ===\r\n");
    printf("[SYSTEM] Trang thai hien tai: STOP\r\n");

    LCD_SetCursor(0, 0);
    LCD_SendString("STOP");
    LCD_SetCursor(1, 0);
    LCD_SendString("T=N/A   ");

    uint8_t data;

    while (1) {
        // 1. Lấy dữ liệu từ Ring Buffer
        while (UART_GetByte(&data)) {
            UART_ProcessByte(data);
        }

        // 2. Xử lý lệnh
        if (flag_new_command) {
            flag_new_command = 0;
            printf("[UART] Nhan duoc: '%s'\r\n", uart_buffer);
            UART_ProcessCommand();
        }

        // 3. Cập nhật LCD
        if (flag_update_lcd_state) {
            flag_update_lcd_state = 0;
            LCD_SetCursor(0, 0);
            if (sys_state == 1) {
                LCD_SendString("RUN ");
            } else {
                LCD_SendString("STOP");
            }
        }

        // 4. Đọc nhiệt độ và chạy Edge AI
        if (flag_read_temp && sys_state == 1) {
            flag_read_temp = 0;
            char lcd_buf[16];
            last_temp = Sensor_ReadTemperature();

            if (last_temp == TEMP_ERROR) {
                LCD_SetCursor(1, 0);
                LCD_SendString("T=ERR   ");
                printf("[SYSTEM] TEMP: ERR\r\n");
            } else {
                // Chạy thuật toán AI
                int is_anomaly = Sensor_DetectAnomaly(last_temp);

                LCD_SetCursor(1, 0);
                if (is_anomaly) {
                    sprintf(lcd_buf, "T=%d! AI", last_temp);
                    LCD_SendString(lcd_buf);
                    printf("[AI] TEMP: %d C, AI: ANOMALY\r\n", last_temp);

                    // HÀNH ĐỘNG CẢNH BÁO: Bật toàn bộ 8 LED
                    for (int i = 1; i <= 8; i++) {
                        System_SetLedState(i, 1);
                    }
                } else {
                    sprintf(lcd_buf, "T=%d C   ", last_temp);
                    LCD_SendString(lcd_buf);
                    printf("[AI] TEMP: %d C, AI: NORMAL\r\n", last_temp);

                    // Tắt toàn bộ 8 LED khi trở về bình thường
                    for (int i = 1; i <= 8; i++) {
                        System_SetLedState(i, 0);
                    }
                }
            }
        }
    }
}

void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) {
        Error_Handler();
    }

    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
    PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}