/**
* @file tim.c
* 
* @brief Timer (TIM) driver implementation for PWM generation
*/
#include "tim.h"
#include "rcc.h"

/**
* @brief Configures a timer for PWM output mode on CH1 and CH3
*
* Enables the TIM3 peripheral clock (hardcoded), sets PSC = 15 and ARR = 999 for
* a 1kHz PWM frequency at 16MHz APB1 clock, configures CH1 and CH3
* in PWM mode 1 with preload enabled, enables both channel outputs, a
* and starts the counter with auto-reload perload enabled
*
* @param tim pointer to timer register map
*
* @note Only configures CH1 and CH# - used for
*       dual-direction motor drive
*       Hardcoded for TIM3 regarledss of tim pointer 
*       passed
*
* @retval None
*/
void TIM_PWM_init(TIM_RegMap_t *tim) {

    if(!tim)
        return;

    /* enable TIM3 clock on APB1 */
    SET_BIT(RCC->APB1ENR, 1);

    /* prescaler and auto-reload for 1kHz PWM at 16MHz APB1 clock */
    tim->PSC = 15;
    tim->ARR = TIM_ARR_VAL;

    tim->CCR1 = 0; 
    tim->CCR3 = 0;
    
    /* CCMR1 - CH1 PWM mode 1, preload enable */
    tim->CCMR1 &= ~(7U << 4);
    tim->CCMR1 |= (6U << 4) | (1U << 3); 
    tim->CCER |= (1U << 0); /* enable CH1 output */

    /* CCMR2 - CH3 PWM mode 1, preload enable */
    tim->CCMR2 &= ~(7U << 4);
    tim->CCMR2 |=  (6U << 4) | (1U << 3);
    tim->CCER  |=  (1U << 8); /* enable CH3 output */

    /* ARPE enalbe (auto-reload preload), counter enable*/
    tim->CR1 |= (1U << 7) | (1U << 0); 
}
