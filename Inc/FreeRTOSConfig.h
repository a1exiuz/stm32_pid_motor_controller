/**
* @file FreeRTOSConfig.h
* @brief FreeRTOS configuration header for STM32 Nucleo-F446RE
*
* Configures the FreeRTOS v11.3.0 kenel for the STM32F446RE running
* at 16MHz HSI with a 1ms tick rate. Enables preemptive scheduling,
* dynamic memory allocation via heap_4, mutexes, semaphores, and 
* stack overflow detection
*
* @note The Cortex-M specific section at the bottom is CRITICAL —
*       do not modify configPRIO_BITS, configKERNEL_INTERRUPT_PRIORITY,
*       or configMAX_SYSCALL_INTERRUPT_PRIORITY unless you fully
*       understand the STM32F446RE interrupt priority model.
*
* @note Any ISR that calls FreeRTOS API functions must have its NVIC
*       priority set to configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY (5)
*       or numerically higher (5-15). Priority 0-4 will cause a hard fault.
*
* @ref FreeRTOS Documentation - Customisation
* @ref STM32F446RE Reference Manual RM0390 - Section 10 (NVIC)
*/
#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Scheduler */
#define configUSE_PREEMPTION                    1
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_TICKLESS_IDLE                 0

/* Clock — set to your actual system clock */
#define configCPU_CLOCK_HZ                      ( 16000000UL )
#define configTICK_RATE_HZ                      ( 1000 )       /* 1ms tick */

/* Task settings */
#define configMAX_PRIORITIES                    ( 7 )
#define configMINIMAL_STACK_SIZE                ( 128 )        /* in words */
#define configMAX_TASK_NAME_LEN                 ( 16 )
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1

#define INCLUDE_vTaskDelay          1
#define INCLUDE_vTaskDelayUntil		1
#define INCLUDE_vTaskDelete         1
#define INCLUDE_vTaskSuspend        1
#define INCLUDE_xTaskGetSchedulerState 1

/* Memory */
#define configTOTAL_HEAP_SIZE                   ( 20 * 1024 )  /* 20KB — adjust as needed */
#define configSUPPORT_STATIC_ALLOCATION         0
#define configSUPPORT_DYNAMIC_ALLOCATION        1

/* Features */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( 2 )
#define configTIMER_QUEUE_LENGTH                10
#define configTIMER_TASK_STACK_DEPTH            ( 256 )

/* Debug */
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            1
#define configASSERT( x ) if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); for(;;); }

/* Co-routines — not used */
#define configUSE_CO_ROUTINES                   0

/* Cortex-M specific — CRITICAL, do not change these */
#define configPRIO_BITS                         4              /* F446RE has 4 priority bits */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Map FreeRTOS interrupt handlers to CMSIS names */
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
