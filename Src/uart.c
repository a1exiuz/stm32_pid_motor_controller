/**
* @file uart.c
*
* @brief UART2 driver for STM32F446RE
*
* Provides blocking UART transmit over USART2 at 115200 8N1
* Configured on PA2 (TX) and PA3 (RX) using alternate function AF7
*
* @note PA2 and PA3 must not be reconfigured elsewhere after calling
*       UART2_init() as this driver owns those pins
*       Not thread-safe, All calls from FreeRTOS tasks must be wrapped
*       with uart_mutex to prevent interleaved outpu from concurrent tasks
*
* @ref STM32F446RE Reference Manual RM0390, Section 30 (USART)
*/
#include <stdlib.h>
#include <string.h>
#include "uart.h"
#include "gpio.h"
#include "motor.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "tasks.h"
#include "nvic.h"

/**
* @brief Initializes USART2 at 115200 buad, 8N1 on PA2 (TX) and PA3 (RX)
*
* Configures PA2/PA3 as AF7 via GPIO_init()
* sets baud rate, and enables transmitter, receiver, RXNE interrupt,
* and the USART peripheral. Enables USART2 IRQ (38)
* in the NVIC at priority 6
* 
* BRR calculation for 115200 buad at 16MHz:
* USARTDIV = 16000000 / (16 * 115200) = 8.68
* Mantissa = 8, Fraction = 0.68 * 16 = 10 (0xA)
*
* @retval None
*/
void UART2_init(void) {
    SET_BIT(RCC->APB1ENR, 17); /* USART2 clock enable */

    /* PA2 - USART2_TX, AF7 */
    GPIO_Config_t PA2 = {
        .port = GPIOA,
        .clock_bit = 0,
        .pin_mask = (1U << 2),
        .pin_num = 2,
        
        .mode = GPIO_MODE_AF,
        .pull = GPIO_NOPULL,
        .type = GPIO_OTYPE_PUSHPULL,
        .speed = GPIO_SPEED_HIGH,
        .alternate_func = 7 /* AF7 = USART 2 */
    };

    /* PA3 — USART2_RX, AF7 */
    GPIO_Config_t PA3 = {
        .port = GPIOA,
        .clock_bit = 0,
        .pin_mask = (1U << 3),
        .pin_num = 3,

        .mode = GPIO_MODE_AF,
        .pull = GPIO_NOPULL,
        .type = GPIO_OTYPE_PUSHPULL,
        .speed = GPIO_SPEED_HIGH,
        .alternate_func = 7 /* AF7 = USART2 */
    };
    
    GPIO_init(&PA2);
    GPIO_init(&PA3);

    /* mantissa = 8, fraction = 10 -> 115200 baud */
    USART2->BRR = (8U << 4) | (10U);

    USART2->CR1 = (1U << 3) | /* TE  TE transmit enable */
                  (1U << 2) | /* RE  RE receive enable */
                  (1U << 5) | /* RXNEIE - interrupt on RXNE */
                  (1U << 13); /* UE  UE USART enable */

    NVIC_enable(38, 6);
}

/**
* @brief Transmits a single character over UART2
*
* Blocks until the transmit data register is empty 
* then writes the character
*
* @param c Character to transmit
*
* @note Not thread-safe, hold uart_mutex when calling from FreeRTOS tasks
*
* @retval None
*/
void UART2_send_char(char c) {
    while(!(USART2->SR & (1U << 7))); /* wait TXE */
    USART2->DR = (uint8_t)c;
}

/**
* @brief Transmits a null-terminated string over UART2
*
* Calls UART2_send_char() for each character until null terminator is reached
*
* @param s Pointer to null-termninated string to transmit
*
* @note Not thread-safe, hold uart_mutex when calling from FreeRTOS tasks
*
* @retval None
*/
void UART2_send_str(const char *s) {
    while(*s) {
        UART2_send_char(*s++);
    }
}

/**
* @brief Transmits a signed 32-bit integer as ASCII digits over UART2
*
* Handles negative values by transmitting a '-' prefix
* Digits are extracted in reverse order into a local buffer
* then transmitted in correct order
*
* @param val Signed integer to transmit
*
* @note Buffer is sized for 10 digits plus sign - sufficent for 
*       the full int32_t range (-2147483648 to 2147483647)
*       Not thread-safe, hold uart_mutex when calling from FreeRTOS tasks
*
* @retval None
*/
void UART2_print_int(int32_t val) {
    char buf[12];
    int i = 0;

    if(val < 0) {
        UART2_send_char('-');
        val = -val;
    }

    if(val == 0) {
        UART2_send_char('0');
        return;
    }

    while(val > 0) {
        buf[i++] = '0' + (val % 10); /* extract digits least significant first */
        val /= 10;
    }

    
    for(int j = i - 1; j >= 0; j--) { /* print in reverse to get correct order */
        UART2_send_char(buf[j]);
    }
}

/**
* @brief Parses a UART command string and queues it for PID_Task
*
* Recognizes "SET <rpm>" and "STOP" commands. On a valid match
* builds a MotorCmd_t and sends it to cmd_queue. Unknown commands
* are silently ignored
*
* @param cmd Null-terminated command string (no trailing newline)
*
* @note Direction is controlled by the sign of <rpm> in "SET <rpm>"
*
* @retval None
*/
void parse_motor_cmd(const char* cmd) {
    MotorCmd_t mc = {0};
    uint8_t valid = 1;

    if(strncmp(cmd, "SET ", 4) == 0) {
        mc.cmd = CMD_SET;
        mc.target_rpm = (float)atoi(cmd + 4);
    }
    else if(strcmp(cmd, "STOP") == 0) {
        mc.cmd = CMD_STOP;
    }
    else {
        valid = 0; /* unknown command */
    }

    if(valid)
        xQueueSend(cmd_queue, &mc, 0);
}

/**
* @brief USART2 global interrupt handler
*
* Fires on RXNE (receive data register not empty) Reads the 
* received byte from DR and forwards it to uart_rx_queue for 
* UART_RX_Task to assemble into command strings
*
* @note Do not call directly - invoked automatically by hardware
*       Reading USART->DR clears the RXNE flag
*
* @retval None
*/
void USART2_IRQHandler(void) {
    if (USART2->SR & (1U << 5)) {
        char c = (char)(USART2->DR & 0xFF);
        xQueueSendFromISR(uart_rx_queue, &c, NULL);
    }
}
