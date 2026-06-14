/**
* @file nvic.c
*
* @brief NVIC (Nested Vectored Interrupt Contrroller) driver for STM32F446RE
*
* Provdies enable and disable control for external interrupts via the 
* Cortex=M4 NVIC registers. Supports IRQs 0-63 across ISER0/ISER1
* and ICER0/ICER1 regisster pairs
*
* @note The STM32F446RE uses 4 priority bits (upper nibble of IPR byte).
*       When used with FreeRTOS, any IRQ that calls FreeRTOS API
*       must have its priority set to 5 or higher (numerically) to avoid a crash
*
* @ref STM32F446RE Reference Manual RM0390, Section 10 (NVIC)
* @ref ARM Cortex-M4 Generic User Guide - Section 4.2 (NVIC)
*/

#include "nvic.h"
#include <stdint.h>

/**
* @brief Enables an IRQ and sets its priority in the NVIC
*
* Writes the priority to the IPR register and sets the corresponding
* bit in ISER0 (IRQ 0-31) or ISER1 (IRQ 32-63) to enable the interrupt
*
* @param irq IRQ number to enable (0-63)
* @param priority Interrupt priority (0-15). Lower value = higher priority.
*                 Stored in the upper 4 bits of the IPR byte
*
* @note On STM32F446RE only the upper 4 bits of each priority
*       byte are implemented. Priority is shifted internally.
*       Assign lower priority (higher number) to non-critical
*       interrupts to avoid preempting time-sensitive ones.
*       When used with FreeRTOS, priority must be >= to configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5)
*       Priority 0-4 will cause a crash if FreeRTOS API is called from the IRQ handler
*
* @retval None
*/
void NVIC_enable(uint8_t irq, uint8_t priority) {
    NVIC_IPR[irq] = priority << 4; /* shift into upper nibble of IPR byte */
                                    /* if you pass priority 1 = 0001 0000 */ 
    if(irq < 32) /* ISER is a bit field — each bit corresponds to one IRQ number. So to enable IRQ23 (EXTI9_5) */
        NVIC_ISER0 |= (1U << irq); /* enable IRQ 0-31 via ISER0*/
    else  /* anything over 32= like irq40 (exti15_10)  - 1U << (40 - 32) = 1U << 8 → bit 8 in ISER1 */
        NVIC_ISER1 |= (1U << (irq - 32)); /* enable IRQ 32-63 via ISER1 */
 }

/**
* @brief Disables an IRQ in the NVIC
*
* Sets the corresponding bit in ICER0 (IRQ 0 - 31) or ICER1 (IRQ 32-63)
* to disable the interrupt. Writing 0 to ICER bits has no effect
*
* @param irq IRQ number to disable (0-63)
*
* @note Writing 1 to an ICER bit disable th IRQ
*      Writing 0 has no effect, so OR assignment is sage
*
* @retval None
*/
void NVIC_disable(uint8_t irq) {
    if(irq < 32)
        NVIC_ICER0 |= (1U << irq); /* disable IRQ 0-31 via ICER0*/
    else                            /* Writing a 1 to a bit in ICER disables that IRQ. Writing a 0 does nothing */
        NVIC_ICER1 |= (1U << (irq - 32)); /* disable IRQ 32-63 via ICER1 */
}

