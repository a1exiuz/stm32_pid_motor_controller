/**
* @file main.c
* 
* @brief Application entry point for STM32F446RE PID motor controller
*
* Initializes peripherals (UART2, motor PWM, encoder), creates FreeRTOS
* IPC primitives, starts Startup_Task, and launches the scheduler
*/

#include "FreeRTOS.h"
#include "tasks.h"
#include "task.h"
#include <stdint.h>
#include "tim.h"
#include "gpio.h"
#include "motor.h"
#include "uart.h"
#include "encoder.h"
#include "exti.h"

/**
* @brief FreeRTOS malloc failed hook — halts on heap exhaustion.
*
* @note Do not call directly — invoked automatically by FreeRTOS kernel.
*/
void vApplicationMallocFailedHook(void) {
    taskDISABLE_INTERRUPTS();
    while(1);
}

/**
* @brief FreeRTOS stack overflow hook — halts on task stack overflow.
*
* @param xTask Task handle of the offending task
* @param pcTaskName Name string of the offending task
*
* @note Do not call directly — invoked automatically by FreeRTOS kernel.
*/
void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
    (void)xTask;
    (void)pcTaskName;
    taskDISABLE_INTERRUPTS();
    while(1);
}

/**
* @brief Application entry point
*
* Initializes UART2 (PA2/PA3 configured internaly by UART2_init())
* motor PWM (TIM3 CH1/CH3), and quadrature encoder (PB6/PA8)
* Creates cmd_queue, uart_rx_queue, and uart_mutex for inter-task
* communication, then creates Startup_Task which spawns all 
* application tasks before starting the FreeRTOS scheduler
*
* @note Never returns - vTaskStartScheduler() takes over execution 
*       The trailing while(1) is unreachable under normal operation
*        and only catches the case where the scheduler fails to start 
*
* @retval int never returns 
*/
int main (void) {
  UART2_init();
  motor_pwm_init();
  encoder_init();
  
  cmd_queue  = xQueueCreate(10, sizeof(MotorCmd_t));
  uart_rx_queue = xQueueCreate(64, sizeof(char));
  uart_mutex = xSemaphoreCreateMutex();
  
  /* Create Startup_Task — all other tasks created from within it */
  configASSERT(xTaskCreate(Startup_Task, "Init", 256, NULL, 4, NULL) == pdPASS);

  vTaskStartScheduler();

  while(1);
}
