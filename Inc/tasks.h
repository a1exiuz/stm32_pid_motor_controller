/**
* @file tasks.h
* @brief FreeRTOS task declarations and shared state for PID motor controller
*
* Defines the application tasks, inter-task communication primitives, 
* and shared volatile state used for UART and PID control
* 
* Task architecture:
*   - PID_Task (priority 4) - runs PID loop every 100ms
*   - UART_RX_Task (priority 3) - parses incoming UART commands
*   - Monitor_Task (priority 2) - sends telemetry every 200ms
*   - Startup_Task - creates all tasks then deletes itself
* 
* @note PID_Task priority must be >= configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY
*       constraints are not relevant here since these are tasks not ISRs
*       but task priorities still reflect real-time requirements -
*       PID_Task is highest priority since it directly drives the motor
*/
#ifndef TASKS_H
#define TASKS_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include <stdint.h>

/** @brief Queue for motor commands sent from UART_RX_Task to PID_Task */
extern QueueHandle_t cmd_queue;

/** @brief Queue for raw UART RX bytes sent from ISR to UART_RX_Task */
extern QueueHandle_t uart_rx_queue;

/** @brief Mutex guarding UART2 transmit - prevents intervlead output */
extern SemaphoreHandle_t uart_mutex;

/** @brief Current measured motor speed in RPM, updated in PID_Task */
extern volatile float actual_rpm;

/** @brief Current commanded target RPM, displayed by Monitor_Task */
extern volatile float target_rpm;

/** @brief Last PID output value (PWM), displayed by Monitor_Task */
extern volatile float pid_output;

/** @brief 1 if motor is actively running under PID control, 0 if stopped */
extern volatile uint8_t motor_running;

/** @brief Ramped target RPM = steps toward commanded_target each cycle */
extern volatile float ramp_target;

/** @brief Fianl target RPM including direction sign, set by CMD_SET */
extern volatile float commanded_target;

/**
* @brief Main PID control loop task
*
* Runs every PID_PERIOD_MS (100ms). Processes CMD_SET/CMD_STOP commands
* from cmd_queue, ramps target RPM toward commanded_target, runs
* PID_update(), and applies output via motor_set()
*
* @param p Unused FreeRTOS task parameter
*
* @note Priority must remain higher than UART_RX_TASK to ensure 
*       consistent 100ms control loop timing isnt delayed by 
*       UART command proccessing 
*
* @retval None
*/
void PID_Task(void* p);

/**
* @brief Parses incoming UART command bytes into motor commands
*
* Receives bytes from uart_rx_queue, buffers until newline/carriage
* return, then parses the buffered string into a MotorCmd_t and
* sends it to cmd_queue for PID_Task
*
* @param p Unused FreeRTOS task paramete
*
* @retval None
*/
void UART_Task(void* p);

/**
* @brief Parses incoming UART command bytes into motor commands
*
* Receives bytes from uart_rx_queue, buffers until newline/carriage
* return, then parses the buffered stirng into a MotorCMD_t and 
* sends it to cmd_queue for PID_Task
*
* @param p Unused FreeRTOS task parameter
*
* @retval None
*/
void Monitor_Task(void* p);

/**
* @brief One-shot startup task that creates all application tasks
*
* Creates PID_Task, UART_RX_Task, and Monitor_Task with configASSERT
* to halt on creation failure, then deletes itself to free its stack
*
* @param p Unused FreeRTOS task parameter
*
* @retval None
*/
void Startup_Task(void* p);

#endif /* TASKS_H */
