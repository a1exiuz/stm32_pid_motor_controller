/**
* @file nvic.h
* @brief NVIC driver for STM32F446RE
*
* Provdies enable and disable control for external interrupts via the 
* Cortex=M4 NVIC registers. Supports IRQs 0-63 across ISER0/ISER1
* and ICER0/ICER1 regisster pairs
*
* @note The NVIC is an ARM core peripheral — register addresses and
*       behavior are identical across all Cortex-M4 devices.
*       When used with FreeRTOS, any IRQ that calls FreeRTOS API
*       must have its priority set to 5 or higher (numerically) to avoid a crash
*
* Base address: 0xE000E100
*
* @ref STM32F446RE Reference Manual RM0390 - Section 10 (NVIC)
* @ref ARM Cortex-M4 Generic User Guide - Section 4.2 (NVIC)
 */

#ifndef NVIC_H
#define NVIC_H

#include <stdint.h>

/**
* @defgroup NVIC_REGS NVIC Register Definitions
* @{
*/
#define NVIC_ISER0  (*(volatile uint32_t*)0xE000E100UL) /**< Interrupt set-enable: IRQ 0-31 */
#define NVIC_ISER1  (*(volatile uint32_t*)0xE000E104UL) /**< Interrupt set-enable: IRQ 32-63 */
#define NVIC_IPR    ((volatile uint8_t*)0xE000E400UL) /**< Priority registers: 1 byte per IRQ */
#define NVIC_ICER0  (*(volatile uint32_t*)0xE000E180UL) /**< Interrupt clear-enable: IRQ 0-31  */
#define NVIC_ICER1  (*(volatile uint32_t*)0xE000E184UL) /**< Interrupt clear-enable: IRQ 32-63 */
/** @} */

/**
* @brief Enables an IRQ and sets its priority in the NVIC.
*
* @param irq IRQ number to enable (0-63)
* @param priority Interrupt priority (0-15). Lower value = higher priority.
*                 Stored in the upper 4 bits of the IPR byte
*
* @note When used with FreeRTOS, priority must be >= to configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5)
*       Priority 0-4 will cause a crash if FreeRTOS API is called from the IRQ handler
*
* Example:
* @code
*   NVIC_enable(6, 5); // enable EXTI0 at priority 5
* @endcode
*
* @retval  None
*/
void NVIC_enable(uint8_t irq, uint8_t priority);

/**
* @brief Disables an IRQ in the NVIC.
*
* @param irq  IRQ number to disable (0-63)
*
* Example:
* @code
*   NVIC_disable(6); // disable EXTI0
* @endcode
*
* @retval None
*/
void NVIC_disable(uint8_t irq);

#endif /* NVIC_H */

