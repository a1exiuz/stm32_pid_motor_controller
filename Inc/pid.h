/**
* @file pid.h
*
* @brief Dicrete PID controller for closed-loop DC motor speed control
*
* Implements a position-form PID controller with anti-windup integral clamping
* and symmetric output clamping. Tuned for N20 6V 500RPM motor wih
* DRV8833 H-bridge driven at 1kHz PWM
* 
* Tuned gains for this system
*   - Kp = 3.5 (proportional - drives convergence speed)
*   - Ki = 0.7 (integral - elimates steady state error)
*   - Kd = 0.05 (derivative - damps post-load overshoot)
*   - integral_max = 350.0f
*
* @ref STM32F446RE Reference Manual RM0390
*/
#ifndef PID_H
#define PID_H

/**
* @brief PID sample period in milliseconds
*
* Must match vTaskDelayUntil period in PID_Task
* Reducing this improves response speed at the cost of CPU load
*/
#define PID_PERIOD_MS  100

/**
* @brief PID sample period in seconds used in PID_update() calculations
*
* Derived from PID_PERIOD_MS. Used for integral accumulation
* and derivative calculation
*/
#define PID_DT         (PID_PERIOD_MS / 1000.0f)  /* 0.1f */

/**
* @brief PID controller state and configuration
*
* One instance per controlled axis. Initialized with PID_init()
* before use. Reset with PID_reset() on new motor command
*/
typedef struct {
    float kp; /**< proportional gain */
    float ki; /**< integral gain */
    float kd; /**< derivative gain */
    float integral; /**< accumulated error */
    float prev_error; /**< last error for derivative */
    float target; /**< target RPM */
    float integral_max; /**< anti-windup clamp */
    float output_max; /**< output clamp (= PWM max) */
} PID_t;

/**
* @brief Initializes PID gains and resets controller state
*
* Sets Kp, Ki, Kd and clears integral, prev_error, and target
* Sets integral_max to 350.0f and output_max to 999.0f
*
* @param pid Pointer to PID_t instance to initialize
* @param kp Proportional gain
* @param ki Integral gain
* @param kd Derivative gain
*
* @retval None
*/
void PID_init(PID_t* pid, float kp, float ki, float kd);

/**
* @brief Computes PID output for one control cycle
*
* Calculates P, I, and D terms from current error, accumulates 
* integral with anti-windup clamping
* Updates prev_error for next derivative calculation
*
* @param pid Pointer to initialized PID_t instance
* @param actual Current measured motor speed in RPM
* @param dt Sample period in seconds (use PID_DT macro)
*
* @note Does not modify gains (kp, ki, kid), target, or output_max
*       Call PID_init() to fully reinitialize the controller
*       dt must match the actual task period - mismatch causese 
*       incorrect integral accumulation and derivative scaling
*
* @retval float PID output in range [-output max, +output max]
*         Pass directly to motor_set() as PWM value.
*/
float PID_update(PID_t* pid, float actual, float dt);

/**
* @brief Resets PID integral and derivative state
*
* Clears integral accumulator and prev_error without touching gains
* or target. Call on CMD_SET and CMD_STOP to prevent stale state
* from corrupting the next control cycle
*
* @param pid Pointer to PID_t instance to reset 
*
* @retval None
*/
void PID_reset(PID_t* pid);

#endif /* PID_H */
