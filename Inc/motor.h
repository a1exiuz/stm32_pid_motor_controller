/**
* @file motor.h
*
* @brief DC motor driver for DRV8833 H-bridge on STM32F446RE
*
* Controls motor direction and speed using dual PWM channels on TIM3
* Forward drive on TIM3_CH1 (PA6), reverse drive on TIM3_CH3 (PB0)
* Eliminates DRV8833 brake mode asymmetry by using PWM on both inputs 
* instead of a GPIO direction pin
*
* Hardware connections:
*   -PA6 (TIM3_CH1, AF2) -> DRV8833 AIN1 (forward PWM)
*   -PB0 (TIM3_CH3, AF2) -> DRV8833 AIN2 (reverse PWM)
*
* @ref STM32F446RE Reference Manual RM0390, Section 18 (TIM3)
* @ref DRV8833 Datasheet, Texas Instruments
*/
#ifndef MOTOR_H
#define MOTOR_H

#include "gpio.h"
#include "tim.h"
#include <stdint.h>

/**
* @brief Minimum PWM value to overcome motor static friction
*
* Below this threshold the motor jitters instead of spinning cleanly
* Determined empirically for the N20 6V 500RPM motor
*/
#define MOTOR_MIN_PWM 150

/**
* @brief Motor command types sent via FreeRTOS queue to PID task
*/
typedef enum {
    CMD_SET,    /**< Set target RPM - payload in MotorCmd_t.target_rpm */
    CMD_STOP,   /**< Stop motor and reset PID state */
} MotorCmdType_t;

/**
* @brief Motor command struct passed through FreeRTOS cmd_queue
*
* Sent from UART_RX_Task to PID_Task via xQueueSend()
*/
typedef struct { 
    float target_rpm; /**< Target speed in RPM. Used with CMD_SET */
    uint8_t cmd; /* CMD_SET, CMD_STOP */
} MotorCmd_t;

/**
* @brief Initializes TIM3 PWM outputs for motor forward and reverse drive
*
* Configures PA6 as TIM3_CH1 (AF2) for forward PWM and 
* PB0 as TIM3_CH3 (AF2) for reverse PWM, then calls 
* TIM_PWM_init() to configure TIM3 frequency, duty cycle
* and enable both PWM channels
* 
* PWM frequency to 1kHz (PSC=15, ARR=999) at 16MHz APB1 clock
* Both channels initialized to 0& duty cycle (motor stopped)
*
* @see TIM_PWM_init()
*
* @retval None
*/
void motor_pwm_init(void);

/**
* @brief Sets motor speed and direction via dula PWM outputs 
*
* Positive speed drives forward via TIM3_CH1 (PA6), sets TIM3_CH3 to 0
* Negative speed drives reverse via TIM3_CH3 (PB0), sets TIM3_CH1 to 0
* Speed magnitued is clamped to [0, 999] mathcing TIM3 ARR value
*
* @param speed PWM duty cycle magnitued with direction sign
*              Range: -999 (full reverse) to +999 (full forward). 0 = coast
*
* @retval None
*/
void motor_set(int32_t speed);

#endif /* MOTOR_H */

