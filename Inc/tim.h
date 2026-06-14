/**
* @file tim.h
*
* @brief Timer (TIM) driver for PWM generation on STM32F446RE
*
* Configures general-purpose timers for PWM output mode
* Used to drive the DRV8833 H-bridge motor inputs via TIM3
* channles 1 to 3
*
* Base addresses:
*   - TIM2: 0x40000000
*   - TIM3: 0x40000400
*   - TIM4: 0x40000800
*   - TIM5: 0x40000C00
*
* @ref STM32F446RE Reference Manual RM0390, SEction 18
*/
#ifndef TIM_H
#define TIM_H

#include <stdint.h>
#include "gpio.h"

/** @brief TIM2 peripheral base address */
#define TIM2 ((TIM_RegMap_t*)0x40000000UL)

/** @brief TIM3 peripheral base address */
#define TIM3 ((TIM_RegMap_t*)0x40000400UL)

/** @brief TIM4 peripheral base address */
#define TIM4 ((TIM_RegMap_t*)0x40000800UL)

/** @brief TIM5 peripheral base address */
#define TIM5 ((TIM_RegMap_t*)0x40000C00UL)

/** 
* @brief Auto-reload register value for PWM period
*
* With PSC = 15 and 16MHz APB1 timer clock, ARR=99 gives 
* a PWM frequency of 1kHz (16MHz / 16 / 1000 = 1kHz)
*/
#define TIM_ARR_VAL 999U

/**
* @brief General-purpose timer register map
*
* Mapped directly to hardware at the timer base address
*/
typedef struct {
    volatile uint32_t CR1; /**< 0x00 — Control register 1 (counter enable, direction) */
    volatile uint32_t CR2; /**< 0x04 — Control register 2 */
    volatile uint32_t SMCR; /**< 0x08 — Slave mode control */
    volatile uint32_t DIER; /**< 0x0C — DMA/interrupt enable register */
    volatile uint32_t SR; /**< 0x10 — Status register */
    volatile uint32_t EGR; /**< 0x14 — Event generation register */
    volatile uint32_t CCMR1; /**< 0x18 — Capture/compare mode register 1 (CH1, CH2) */
    volatile uint32_t CCMR2; /**< 0x1C — Capture/compare mode register 2 (CH3, CH4) */
    volatile uint32_t CCER; /**< 0x20 — Capture/compare enable register */
    volatile uint32_t CNT; /**< 0x24 — Counter register */
    volatile uint32_t PSC; /**< 0x28 — Prescaler register */
    volatile uint32_t ARR; /**< 0x2C — Auto-reload register (PWM period) */
    volatile uint32_t RESERVED[1]; /**< 0x30 — Reserved (RCR, not used) */
    volatile uint32_t CCR1; /**< 0x34 — Capture/compare register 1 (CH1 duty cycle) */
    volatile uint32_t CCR2; /**< 0x38 — Capture/compare register 2 (CH2 duty cycle) */
    volatile uint32_t CCR3; /**< 0x3C — Capture/compare register 3 (CH3 duty cycle) */
    volatile uint32_t CCR4; /**< 0x40 — Capture/compare register 4 (CH4 duty cycle) */
} TIM_RegMap_t;

/**
* @brief Configures a timer for PWM output mode on CH1 and CH3
*
* Enables the TIM3 peripheral clock (APB1ENR bit 1), sets PSC = 15
* and ARR=999 for 1kHz PWM frequnecy, configures CH1 and CH3 in 
* PWM mode 1 with preload enabled, enables both channel outputs,
* and starts the counter with auto-reload perload enabled
*
* @param tim Pointer to timer register map (e.g. TIM3)
*
* @note Only configures CH1 and CH3 - used for dual-direction
*       motor drive (forward/reverse)
*       The peripheral clock enable is hardcoded to TIM3
*       This function is currently TIM3-specific despite its
*       generic signature
*
* @retval None
* 
*/
void TIM_PWM_init(TIM_RegMap_t *tim);

#endif /* TIM_H */



