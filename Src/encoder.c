/**
* @file encoder.c
*
* @brief Quadrature encoder driver implementation
*
* @see encoder.h for hardware connections and public API documentation
*/
#include "encoder.h"
#include "rcc.h"
#include "gpio.h"
#include "exti.h"

/**
* @brief Running pulse count, incremented/decremented by EXTI9_5_IRQHandler
*
* Increments on forward rotation, decrement on reverse
*/
volatile int32_t encoder_counter = 0; /* updated in isr */

/**
* @brief Previous encoder count, used to compute delta in encoder_get_rpm()
*
* Internal to this file 
*/
static int32_t last_count = 0;

/**
* @brief Initialized encoder GPIO inputs and EXTI interrupts
*
* Configures PB6 (channel A) and PA8 (channel B) as floating inputs
* then calls EXTI_init_encoder() to configure quadrature decoding
*
* @retval None
*/
void encoder_init(void) {
    /* Configure PB6 and PA8 as inputs with pull-downs for encoder channels A and B */
    GPIO_Config_t PB6 = {
        .port = GPIOB,
        .clock_bit = 1,
        .pin_mask = (1U << 6),
        .pin_num = 6,

        .mode = GPIO_MODE_INPUT,
        .pull = GPIO_NOPULL,
        .type = GPIO_OTYPE_PUSHPULL,
        .speed = GPIO_SPEED_LOW,
        .alternate_func = 0
    };

    GPIO_Config_t PA8 = {
        .port = GPIOA,
        .clock_bit = 0,
        .pin_mask = (1U << 8),
        .pin_num = 8,

        .mode = GPIO_MODE_INPUT,
        .pull = GPIO_NOPULL,
        .type = GPIO_OTYPE_PUSHPULL,
        .speed = GPIO_SPEED_LOW,
        .alternate_func = 0
    };

    GPIO_init(&PB6);
    GPIO_init(&PA8);
    EXTI_init_encoder(); /* configure EXTI for encoder pins */
}

/**
* @brief Calculates motor output shaft speed in RPM
* 
* Computes delta between current and previous encoder count
* scales to counts/min, then divides by COUNTS_PER_REV to get RPM
*
* @note This calculation assumes encoder_get_rpm() is called 
*       exatcly once per PID_DT seconds. Calling it at a different 
*       raten than PID_Tasks period will produce incorrect RPM values
*
* @retval float Current motor speed in RPM. Negative = reverse
*/
float encoder_get_rpm(void) {
    int32_t current = encoder_counter; /* read current count atomically by reading the volatile variable */ 
    int32_t delta = current - last_count;
    last_count = current;

    /* delta counts in 100ms -> scale to counts/min -> divide by CPR*/
    return (delta * 10.0f * 60.0f) / COUNTS_PER_REV;
}

/**
* @brief Resets encoder counters to zero
*
* Clears encoder_counter and last_count
*
* @note Must be called on CMD_SET and CMD_STOP to prevent stale 
*       count deltas from corrupting the first RPM reading after
*       a direction change or motor restart
*
* @retval None
*/
void encoder_reset(void) {
	encoder_counter = 0;
	last_count = 0;
}
