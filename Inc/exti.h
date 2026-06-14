/** 
* @file exti.h
*
* @brief External Interrupt (EXTI) driver for STM32F446RE
*
* Configures GPIO pins as external  interrupt sources with 
* rising and falling edge detection for quadrature encoder decoding
*
*    Base addresses:
*       - EXTI: 0x40013C00
*       - SYSCFG: 0x40013800
* @ref STM32F446RE Reference Manual RM0390, Section 12 (EXTI)
*/

#ifndef EXTI_H
#define EXTI_H

#include <stdint.h>

/** @brief EXTI peripheral base address */
#define EXTI ((EXTI_RegMap_t*)0x40013C00UL)

/** @brief SYSCFG peripheral base address */
#define SYSCFG ((SYSCFG_RegMap_t*)0x40013800UL)

/**
* @defgroup EXTI_IRQ EXTI IRQ Numbers
*
* @brief NVIC IRQ numbers for EXTI llines
* 
* Pins 0-4 have dedicated IRQs, pins 5-9 and 10-15 share IRQs.
*@{
*/
#define EXTI0_IRQn       6      /**< pin 0 - dedicated IRQ */
#define EXTI1_IRQn       7      /**< pin 1 - dedicated IRQ */
#define EXTI2_IRQn       8      /**< pin 2 - dedicated IRQ */
#define EXTI3_IRQn       9      /**< pin 3 - dedicated IRQ */
#define EXTI4_IRQn       10     /**< pin 4 - dedicated IRQ */
#define EXTI9_5_IRQn     23     /**< pins 5-9 - shared IRQ, check PR in handler */
#define EXTI15_10_IRQn   40     /**< pins 10-15 - shared IRQ, check PR in handler */
/** @} */

/** 
* @brief EXTI peripheral register map
*
* Controls interrupt masking, edge detection, and pending flags
* Mapped directly to hardware at base address 0x40013C00
*/
typedef struct {
    volatile uint32_t IMR;          /**< 0x00 - interrupt mask (1=unmasked/enabled) */
    volatile uint32_t RESERVED1[1]; /**< 0x04 - reserved (EMR even mask, not used) */
    volatile uint32_t RTSR;         /**< 0x08 - rising edge trigger select */
    volatile uint32_t FTSR;         /**< 0x0C - falling edge trigger select */
    volatile uint32_t RESERVED2[1]; /**< 0x10 - resereved (SWIER software interrupt) */
    volatile uint32_t PR;           /**< 0x14 - pending register (write 1 to clear) */
} EXTI_RegMap_t;

/** 
* @brief SYSCFG peripheral register map (partial)
*
* EXTICR registers map GPIO ports to EXTI lines
* Only EXTICR fields included - other SYSCFG registers not used
*/
typedef struct {
    volatile uint32_t RESERVED2[2]; /**< 0x00-0x04 - MEMRMP, PMC (not used) */
    volatile uint32_t EXTICR[4];    /**< 0x08-0x14 - EXTI port selection (4 bits per pin) */
    volatile uint32_t RESERVED3[2]; /**< 0x18-0x1C - reserved */
} SYSCFG_RegMap_t;

/**
* @brief Configures EXTI interrupts for quadrature encoder channels
*
* Maps PB6 (channel A) and PA8 (channel B) to EXTI lines 6 and 8
* Enables both rising and falling edges on both channels for 
* maximum 4x quadrature decoding resolution
* Enables EXTI9_5_IRQn in NVIC at priority 5
*
* @retval None
*/
void EXTI_init_encoder(void);

/**
* @brief EXTI lines 9-5 shared interrupt handler
*
* Handles quadrature encoder pulses on PB6 (channel A, EXTI6)
* and PA8 (channel B, EXTI8). Determines rotation direction by 
* reading the complementary channel state on each edge, then 
* increments or decrements encoder_counter accordingly
*
* @note Do not call directly - invoked automatically by hardware
*
* @retval None 
*/
void EXTI9_5_IRQHandler(void);  

#endif /* EXTI_H */
