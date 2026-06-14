/**
* @file tasks.c
*
* @brief FreeRTOS task implemetations for PID motor controller
*/
#include "tasks.h"
#include "uart.h"
#include "pid.h"
#include "encoder.h"
#include "motor.h"

QueueHandle_t cmd_queue;
QueueHandle_t uart_rx_queue;
SemaphoreHandle_t uart_mutex;
volatile float actual_rpm = 0.0f;
volatile float target_rpm = 0.0f;
volatile float pid_output = 0.0f;
volatile float ramp_target = 0.0f;
volatile float commanded_target = 0.0f;
volatile uint8_t motor_running = 0;

/**
* @brief Main PID control loop task
*
* Runs every 100ms, Processes CMD_SET and CMD_STOP from cmd_queue
* ramps target RPM toward commanded_target in steps of 30 RPM/cycle
* runs PID_update(), and applies output via motor_set()
*
* @param p Unused FreeRTOS task parameter
*
* @note Direction is controlled via the sign of the SET command
*
* @retval None 
*/
void PID_Task(void* p) {
    (void)p;
    PID_t pid;
    PID_init(&pid, 3.5f, 0.7f, 0.05f); /* final tuned gains */
    TickType_t xLastWake = xTaskGetTickCount();
    MotorCmd_t cmd;

    while (1) {
        if(xQueueReceive(cmd_queue, &cmd, 0) == pdPASS) {
            switch(cmd.cmd) {
                case CMD_SET:
                    commanded_target = cmd.target_rpm; 
                    target_rpm = commanded_target;
                    ramp_target = 0.0f; /* reset ramp from 0 */
                    pid.target = 0.0f;
                    motor_running = 1;
                    encoder_reset();
                    PID_reset(&pid);

                    xSemaphoreTake(uart_mutex, portMAX_DELAY);
                    UART2_send_str("[PID] CMD_SET received\r\n");
                    xSemaphoreGive(uart_mutex);
                    break;

                case CMD_STOP:
                    pid.target = 0.0f;
                    target_rpm = 0.0f;
                    motor_running = 0;
                    motor_set(0);
                    PID_reset(&pid);
                    encoder_reset();
                    xSemaphoreTake(uart_mutex, portMAX_DELAY);
                    UART2_send_str("[PID] CMD_STOP received\r\n");
                    xSemaphoreGive(uart_mutex);
                    break;

                default:
                    break;
            }
        }
        if(motor_running) {
            /* ramp toward commanded_target at 30 RPM per 100ms cycle */
            if(commanded_target > 0) {
                if(ramp_target < commanded_target) {
                    ramp_target += 30.0f;
                    if(ramp_target > commanded_target) ramp_target = commanded_target;
                }
            } else {
                if(ramp_target > commanded_target) {
                    ramp_target -= 30.0f;
                    if(ramp_target < commanded_target) ramp_target = commanded_target;
                }
            }

            pid.target = ramp_target;

            float rpm = encoder_get_rpm();
            float output = PID_update(&pid, rpm, PID_DT);

            motor_set((int32_t)output);

            actual_rpm = rpm;
            pid_output = output;
        }

        vTaskDelayUntil(&xLastWake, pdMS_TO_TICKS(100));
    }
}

/**
* @brief Parses incoming UART command bytes into motor commands
*
* Receives bytes from uart_rx_queue, buffers until newline/carriage
* return, then passes the buffered string to parse_motor_cmd(),
* which queues a MotorCmd_t to cmd_queue for PID_Task
*
* @param p Unused FreeRTOS task parameter
*
* @retval None
*/
void UART_RX_Task(void* p) {
    (void)p;
    char cmd_buf[32];
    uint32_t cmd_len = 0;
    char c;

    while (1) {
        if (xQueueReceive(uart_rx_queue, &c, portMAX_DELAY) == pdPASS) {
            if(c == '\n' || c == '\r') {
                cmd_buf[cmd_len] = '\0';
                if (cmd_len > 0) {
                    parse_motor_cmd(cmd_buf);
                }
                cmd_len = 0;
            } else if (cmd_len < 31) {
                cmd_buf[cmd_len++] = c;
            }
        }
    }
}

/**
* @brief Telemmetry output taks
*
* Runs every 200ms. Sends target RPM, actual RPM, PID output (PWM)
* and error over UART2 in the format:
* "TGT:<val> ACT:<val> PWM:<val> ERR:<val>"
*
* @param p Unused FreeRTOS task parameter
*
* @retval None
*/
void Monitor_Task(void* p) {
     (void)p;
     
     while(1) {
        vTaskDelay(pdMS_TO_TICKS(200));

        xSemaphoreTake(uart_mutex, portMAX_DELAY);
        UART2_send_str("TGT:");
        UART2_print_int((int32_t)target_rpm);
        UART2_send_str(" ACT:");
        UART2_print_int((int32_t)actual_rpm);
        UART2_send_str(" PWM:");
        UART2_print_int((int32_t)pid_output);
        UART2_send_str(" ERR:");
        UART2_print_int((int32_t)(target_rpm - actual_rpm));
        UART2_send_str("\r\n");
        xSemaphoreGive(uart_mutex);
     }
}

/**
* @brief One-shot startup task that creates all application tasks
*
* Cretaes PID_Task, UART_RX_Task, and Monitor_Task with configASSERT
* to halt on createion failure, then deletes itself to free its stack
*
* @param p Unused FreeRTOS task parameter
*
* @note Must be the first (and only) task created from main() before
*       vTaskStartScheduler(). Deletes itself after spawning the 
*       other tasks, so it only runs once
*
* @retval None
*/
void Startup_Task(void* p) {
    xSemaphoreTake(uart_mutex, portMAX_DELAY);
    UART2_send_str("Startup_Task");
    xSemaphoreGive(uart_mutex);

     /* create all application tasks — configASSERT halts on failure */
    configASSERT(xTaskCreate(PID_Task, "PID_Task", 256, NULL, 4, NULL) == pdPASS);
    configASSERT(xTaskCreate(UART_RX_Task, "UART_RX_Task", 512, NULL, 3, NULL) == pdPASS);
    configASSERT(xTaskCreate(Monitor_Task, "Monitor_Task", 256, NULL, 2, NULL) == pdPASS);
    
    vTaskDelete(NULL); /* free stack memory — task no longer needed */
}
