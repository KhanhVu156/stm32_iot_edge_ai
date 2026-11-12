/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    stm32f1xx_it.c
 * @brief   Interrupt Service Routines.
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_it.h"

#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */
/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;  // Khai bao UART handle

/* Khai bao ham xu ly byte tu app_uart.c */
extern void UART_PushByte(uint8_t data);

/* USER CODE BEGIN EV */
/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/

void NMI_Handler(void) {
    while (1) {
    }
}

void HardFault_Handler(void) {
    while (1) {
    }
}

void MemManage_Handler(void) {
    while (1) {
    }
}

void BusFault_Handler(void) {
    while (1) {
    }
}

void UsageFault_Handler(void) {
    while (1) {
    }
}

void SVC_Handler(void) {}

void DebugMon_Handler(void) {}

void PendSV_Handler(void) {}

void SysTick_Handler(void) { HAL_IncTick(); }

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/******************************************************************************/

void EXTI0_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0); }

void EXTI1_IRQHandler(void) { HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_1); }

void TIM2_IRQHandler(void) { HAL_TIM_IRQHandler(&htim2); }

/**
 * @brief This function handles USART1 global interrupt.
 *
 * FIX (so voi ban cu): ORE va RXNE phai duoc xu ly bang MOT LAN DOC DR DUY
 * NHAT. Theo RM0008 muc 27.6.1:
 *   - Doc USART_SR roi doc USART_DR => clear co ORE
 *   - Doc USART_DR                 => clear co RXNE
 * => Chi can mot lan doc DR la clear ca hai co.
 *
 * Khi ORE=1: DR van chua BYTE HOP LE CUOI CUNG (byte nhan duoc truoc khi tran).
 * Byte gay tran (byte tiep theo) bi mat vinh vien - day la han che phan cung
 * cua STM32F1, khong the khoi phuc. Nhung it nhat phai push byte hop le vao
 * Ring Buffer thay vi vut bo roi push rac (0x00) nhu code cu.
 *
 * Code cu (co bug):
 *   if (sr & ORE)  { (void)DR; }        // vut byte hop le
 *   if (sr & RXNE) { data = DR; push; } // DR rong -> push rac
 * => Mat 2 ky tu moi lan ORE (byte that + byte rac) -> gay loi "LED ON 01"
 *    thanh "LED O 1" khi go nhanh tren TEP Monitor cua Proteus.
 */
void USART1_IRQHandler(void) {
    // Doc SR mot lan duy nhat (snapshot) de tranh race condition
    uint32_t sr = USART1->SR;

    // Neu co bat ky su kien nao (nhan duoc byte moi hoac tran bo dem),
    // doc DR dung MOT lan duy nhat. Byte tra ve la byte hop le cuoi cung.
    // Doc DR se tu dong clear ca RXNE lan ORE (do SR da duoc doc truoc do).
    if (sr & (USART_SR_RXNE | USART_SR_ORE)) {
        uint8_t data = (uint8_t)(USART1->DR & 0xFF);
        UART_PushByte(data);
    }
}

/* USER CODE BEGIN 1 */
/* USER CODE END 1 */