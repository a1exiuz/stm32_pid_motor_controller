/**
* @file pid.c
*
* @brief Discrete PID controller implementation
*/

#include "pid.h"

/**
* @brief Initialized PID gains and resets controller state
*
* Sets Kp, Ki, Kd and clears integral, prev_error, and target
* Sets integral_max to 350.0f and output_max to 999.0f
*
* @param pid Pointer to PID_t instance to initialize
* @param kp Proportional gain
* @param ki integral gain
* @param kd Derivative gain
*
* @retval None
*/
void PID_init(PID_t* pid, float kp, float ki, float kd) {
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->target = 0.0f;
    pid->integral_max = 350.0f; /* prevent integral windup */
    pid->output_max = 999.0f; /* max PWM value */
}

/** 
* @brief Computes PID output for one control cycle
* 
* Calculates P, I, and D terms from current error, accumulates
* integral with anti-windup clamping, and returns clamped output
* Updates prev_error for next derivative calculation
*
* @param pid Pointer to initialized PID_t instance
* @param actaul Current measured motor speed in RPM
* @param dt Sampple period in seconds (use PID_DT macro)
*
* @note dt must be nonzero - a dt of 0 causes division by zero 
*       in the derivative term. dt must also match the actual
*       task period (PID_PERIOD_MS): a mismatch causes incorrect
*       integral accumulation and derivative scaling
*
* @retval float PID ouput in range [-output_max, +output_max]
*               Pss directly to motor_set() as PWM value
*/
float PID_update(PID_t* pid, float actual, float dt) {
    float error = pid->target - actual;

    /* P term */
    float P = pid->kp * error;

    /* I term with anti windup clamp */
    pid->integral += error * dt;
    if(pid->integral > pid->integral_max)
        pid->integral = pid->integral_max;
    if(pid->integral < -pid->integral_max)
        pid->integral = -pid->integral_max;
    float I = pid->ki * pid->integral;

    /* D term */
    float D = pid->kd * ((error - pid->prev_error) / dt);
    pid->prev_error = error;

    /* sum and clamp output */
    float output = P + I + D;
    if(output > pid->output_max)
        output = pid->output_max;
    if(output < -pid->output_max)
        output = -pid->output_max;

    return output;
}

/**
* @brief Resets PID integral and derivative state
* 
* Clears integral accumulator and prev_error without touching gains
* or target
* 
* @param pid Pointer to PID_t instance to reset
* 
* @note Does not modify kp, ki, kd, target, integral_max, or
*       output_max. Call PID_init() to fully reinitialize
*
* @retval None
*/
void PID_reset(PID_t* pid) {
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
}
