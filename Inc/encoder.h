/**
* @file encoder.h
* 
* @brief Quadrature encoder interfeace for N20 DC gear motor
*
* Provides pulse counting via EXTI interrupts on PB6 and PA8
* and RPM calculation for closed-loop PID motor speed control
*
* Hardware: N20 6V 500RPM motor with quadrature encoder
* Encoder channels connected to PB6 (CH A) and PA8 (CH B)
*/

#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

/** @brief Raw pulses per revolution from the encoder (pre-gearbox) */
#define ENCODER_PPR 7 

/** @brief Motor gearbox reduction ratio */
#define GEAR_RATIO 30

/** 
* @brief Total encoder counts per output shaft revolution
* 
* Calculated as: ENCODER_PPR * GEAR_RATIO * 4 (quadrature edges)
* = 7 * 30 * 4 = 840. Empirically measured as 835
*/
#define COUNTS_PER_REV 835 /* quadrature decoding */

/** @brief Maximum rated motor output shaft speed in RPM */
#define MAX_RPM 600

/**
* @brief Running pulse count incremented/decrementd by EXTI ISR
*
* Increments on forward rotation, decrements on reverse
*/
extern volatile int32_t encoder_counter; 

/**
* @brief Initializes encoder GPIO inputs and EXTI interrupts
*
* Configures both channels for both edges to maximazie resolution
* Enables EXTI9_5 IRQ for quadrature decoding 
*/
void encoder_init(void);

/**
* @brief Calculates motor output shaft speed in RPM
*
* Computes delta between current and previous encoder count
* converst to RPM based on PID_PERIOD_MS and COUNTS_PER_REV
* Negative return value indicates reverse rotation
*
* @retval float Current motor speed in RPM. Negative = reverse
*/
float encoder_get_rpm(void);

/**
* @brief Resets encoder counters to zero
*
* Clears both encoder_counter and last_count
* Must be called on CMD_SET and CMD_STOP to prevent stale
* count deltas from corrupting the first RPM reading after
* a direction change or motor restart
* 
* @retval none
*/
void encoder_reset(void);

#endif /* ENCODER_H */
