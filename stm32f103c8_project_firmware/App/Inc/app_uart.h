#ifndef __APP_UART_H
#define __APP_UART_H

#include <stdio.h>
#include <string.h>

#include "main.h"
#include "usart.h"

#define UART_BUFFER_SIZE 64

extern volatile uint8_t sys_state;
extern int last_temp;
extern void System_SetLedState(uint8_t led_num, uint8_t state);

void UART_Init(void);
uint8_t UART_GetByte(uint8_t* data);
void UART_ProcessByte(uint8_t rx_byte);
void UART_ProcessCommand(void);

#endif