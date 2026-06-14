/**
* @file  uart.h
*
* @brief UART2 driver for STM32F446RE
*
* Provides blocking UART transmit over USART2 at 115200 8N1.
* Configured for debug output over the ST-Link virtual COM port.
*
* Pin mapping (fixed by Nucleo hardware, wired to ST-Link):
*   - TX : PA2 (AF7)
*   - RX : PA3 (AF7)
*
* Base address: 0x40004400
*
* @note Not thread-safe, All calls from FreeRTOS tasks must be wrapped
*       with uart_mutex to prevent interleaved outpu from concurrent tasks
*
* @ref STM32F446RE Reference Manual RM0390 - Section 30 (USART)
*/

#ifndef UART_H
#define UART_H

#include <stdint.h>

/** @brief USART2 peripheral base address */
#define USART2 ((USART_RegMap_t*)0x40004400UL)

/**
* @brief USART peripheral register map.
*
* Mapped directly to hardware at base address 0x40004400.
* Each register is 32 bits wide.
*/
typedef struct {
    volatile uint32_t SR;   /**< 0x00 - Status register (TXE, TC, RXNE, ORE flags)      */
    volatile uint32_t DR;   /**< 0x04 - Data register (write = TX, read = RX)            */
    volatile uint32_t BRR;  /**< 0x08 - Baud rate register (mantissa and fraction)       */
    volatile uint32_t CR1;  /**< 0x0C - Control 1 (UE, TE, RE, word length)              */
    volatile uint32_t CR2;  /**< 0x10 - Control 2 (stop bits, clock)                     */
    volatile uint32_t CR3;  /**< 0x14 - Control 3 (DMA, flow control, smartcard)         */
    volatile uint32_t GTPR; /**< 0x18 - Guard time and prescaler (smartcard, IrDA)        */
} USART_RegMap_t;

/**
* @brief Initializes USART2 at 115200 baud, 8N1 on PA2 (TX) and PA3 (RX).
*
* Configures PA2/PA3 as AF7 via GPIO_init() (which also enables the
* GPIOA clock), enables the USART2 clock, sets BRR for 115200 baud
* at 16MHz APB1 clock, and enables transmitter, receiver, RXNE
* interrupt, and the USART peripheral. Enables USART2 IRQ (38)
* in the NVIC at priority 6.
*
* @note Must be called before any UART2 transmit functions.
*       or before command input via uart_rx_queue is available
*
* @retval None
*/
void UART2_init(void);

/**
* @brief Transmits a single character over UART2
*
* Blocks until the transmit data register is empty 
* then writes the character.
*
* @param c Character to transmit
*
* @note Not thread-safe, hold uart_mutex when calling from FreeRTOS tasks
*
* @retval None
*/
void UART2_send_char(char c);

/**
* @brief Transmits a null-terminated string over UART2.
*
* @param s Pointer to null-terminated string to transmit
*
* @note Not thread-safe, hold uart_mutex when calling from FreeRTOS tasks
*
* @retval None
*/
void UART2_send_str(const char *s);

/**
* @brief Transmits a signed 32-bit integer as ASCII digits over UART2.
*
* Handles negative values by transmitting a '-' prefix.
*
* @param val Signed integer to transmit
*
* @note Not thread-safe, hold uart_mutex when calling from FreeRTOS tasks
*
* @retval None
*/
void UART2_print_int(int32_t val);

/**
* @brief Parses a UART command string and queues it for PID_Task
*
* Recognizes "SET <rpm>", "STOP"
* On a valid match, builds a MotorCmd_t and sends it to cmd_queue
* Unknown commands are silently ignored
*
* @param cmd Null-termindated command string (no trailing newline)
*
* @note Called from UART_RX_Task after a full line has been buffered
*
* @retval None
*/
void parse_motor_cmd(const char* cmd);

/**
* @brief USART2 global interrupt handler
*
* Fires on RXNE (recieve data register not empty). Reads the
* received byte from DR and forwards it to uart_rx_queue for 
* UART_RX_Task to assemble into command strings
*
* @note Do not call direclty - invoked automatically by hardware
*       Reading USART2->DR clears the RXNE flag
*
* @retval None
*/
void USART2_IRQHandler(void);

#endif /* UART_H */