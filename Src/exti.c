/**
* @file exti.c
* @brief External Interrupt (EXTI) driver for STM32F446RE
*
* Configures GPIO pins as external interrupt sources with falling edge detection.
* Supports pins 0-15 across all GPIO ports via SYSCFG EXTICR mapping.
* IRQ routing: EXTI0-4 have dedicated handlers, EXTI5-9 and EXTI10-15 each share handlers 
*
* Three button inputs are supported via flag-based polling:
*   - Pin 0 (EXTI0)    : actively used, see EXTI0_IRQHandler
*   - Pin 4 (EXTI4)    : available for future use (e.g. reset, confirm)
*   - Pin 7 (EXTI9_5)  : available for future use (e.g. pattern change, back)
* To use pin 4 or 7, wire a button to the pin and poll EXTI_get_flag2()
* or EXTI_get_flag1() in your main loop.
*
* @note    EXTI0_IRQHandler integrates with FreeRTOS via xSemaphoreGiveFromISR.
*          NVIC priority for PA0 must be >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5).
*
*@ref STM32F446RE Reference Manual RM0390, Section 12 (EXTI)
*/
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "exti.h"
#include "nvic.h"
#include "tasks.h"
#include "rcc.h"
#include "gpio.h"
#include "uart.h"
#include "encoder.h"
#include <stdint.h>

/**
* @brief Configures EXTI interrupts for quadrature encoder channels
*
* Maps PB6 (channel A) to EXTI lines 6 and PA8 (channel B) to EXTI
* lin 8 via SYSCFG_EXTICR. Enables both rising and falling edge
* triggers on both lines for 4x quadrature decoding resolution
* Enables EXTI9_5_IRQn in the NVIC at priority 5
*/
void EXTI_init_encoder(void) {
    SET_BIT(RCC->APB2ENR, 14); /* enable SYSCFG clock */

    /* EXTI6 → Port B (PB6) */
    SYSCFG->EXTICR[1] &= ~(0x0F00U);
    SYSCFG->EXTICR[1] |=  (0x0100U);

    /* EXTI8 -> Port A (PA8)*/
    SYSCFG->EXTICR[2] &= ~(0x000FU);
    // leave as 0 for Port A

    /* Both channels rising edge only */
    EXTI->IMR  |=  (1U<<6) | (1U<<8);
    EXTI->RTSR |=  (1U<<6) | (1U<<8);
    EXTI->FTSR |=  (1U<<6) | (1U<<8);

    NVIC_enable(23, 5);
}


/**
* @brief EXTI lines 5-9 shared interrupt handler (currently handles pin 7).
*
* Handles quadrature encoder pulses on PB6 (channel A, EXTI6)
* and PA8 (channel B, EXTI8). On each edge, read the 
* complementary channels current state to determine rotation
* direction and increments or decrements encoder_counter
*
* @note Do not call directly — invoked automatically by hardware.
*       This handler is shared between pins 5-9.
*       Pending flag cleared by writing 1 to EXTI->PR.
*
* @retval  None
*/
void EXTI9_5_IRQHandler(void) {
        if(EXTI->PR & (1U << 6)) { /* channel A edge */
        EXTI->PR = (1U << 6);
        /* on rising edge: B high = reverse, B low = forward
           on falling edge: B high = forward, B low = reverse */
        if(GPIOB->IDR & (1U << 6)) {  /* A is HIGH = rising edge */
            if(GPIOA->IDR & (1U << 8)) encoder_counter--;
            else                       encoder_counter++;
        } else { /* A is LOW = falling edge */
            if(GPIOA->IDR & (1U << 8)) encoder_counter++;
            else                       encoder_counter--;
        }
    }
    if(EXTI->PR & (1U << 8)) {  /* channel B edge */
        EXTI->PR = (1U << 8);
        if(GPIOA->IDR & (1U << 8)) {  /* B is HIGH = rising edge */
            if(GPIOB->IDR & (1U << 6)) encoder_counter++;
            else                       encoder_counter--;
        } else { /* B is LOW = falling edge */
            if(GPIOB->IDR & (1U << 6)) encoder_counter--;
            else                       encoder_counter++;
        }
    }
}

