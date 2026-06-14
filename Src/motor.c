/**
* @file motor.c
* 
* @brief DC motor driver implementation for DRV8833 H-bridge
*/
#include "motor.h"
#include "uart.h"

/**
* @brief Initializes GPIO pins for motor PWM and delegates timer configuration
*
* Configures PA6 as TIM3_CH1 (AF2) for forward PWM and PB0 as 
* TIM3_CH3 (AF2) for reverse PWM, then calls TIM_PWM_init() to 
* configure TIM3 frequency, duty cycle resolution, and enable 
* both PWM channels
*
* @retval None
*/
void motor_pwm_init(void) {
  /* PA6 - TIM3_CH1 AF2: forward PWM output */
     GPIO_Config_t PA6 = {
    .port = GPIOA,
    .clock_bit = 0,
    .pin_mask = (1U << 6),
    .pin_num = 6,

    .mode = GPIO_MODE_AF,
    .pull = GPIO_NOPULL,
    .type = GPIO_OTYPE_PUSHPULL,
    .speed = GPIO_SPEED_HIGH,
    .alternate_func = 2 /* AF2 = TIM3_CH1 */
  };

  /* PB0 - TIM3_CH3 AF2: reverse PWM output */
  GPIO_Config_t PB0 = {
    .port = GPIOB,
    .clock_bit = 1,
    .pin_mask = (1U << 0),
    .pin_num = 0,

    .mode = GPIO_MODE_AF,
    .pull = GPIO_NOPULL,
    .type = GPIO_OTYPE_PUSHPULL,
    .speed = GPIO_SPEED_HIGH,
    .alternate_func = 2 /* AF2 = TIM3_CH3 */
  };

  GPIO_init(&PA6);
  GPIO_init(&PB0);

  /* configure TIM3 CH1/CH3 PWM mode, frequency, and enable counter */
  TIM_PWM_init(TIM3);
} 

/**
* @brief Sets motor speed and direction via dual PWM outputs
*
* Positive speed drives forward via TIM3_CH1 (PA6) with TIM3_CH3
* forced to 0. Negative speed drives reverse via TIM3_CH3 (PB0)
* with TIM3_CH1 forced to 0. Speed magnitued is clamped to 
* [0, 999] matching TIM3's ARR value
*
* @param speed PWM duty cycle magnitured with direction sign
*              Range: -999 (full reverse) t0 +999 (full forward)
*              0 = both channels off
* @retval None 
*/
void motor_set(int32_t speed) {
    if(speed > 999)  speed = 999;
    if(speed < -999) speed = -999;

    if(speed >= 0) {
        TIM3->CCR3 = 0; /* reverse off */
        TIM3->CCR1 = (uint32_t)speed; /* forward PWM */
    } else {
        TIM3->CCR1 = 0; /* forward off */
        TIM3->CCR3 = (uint32_t)(-speed); /* reverse PWM */
    }
}
