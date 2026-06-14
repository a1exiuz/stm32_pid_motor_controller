/**
* @file gpio.c
* @brief GPIO driver for STM32F446RE
*
* Provides initialization and control functions for GPIO pins using a configuration
* struct. Supports output, input, and alternate function modes with
* configurable pull, speed, and output type
*
* @ref STM32F446RE Reference Manual RM0390
*/
#include "GPIO.H"
#include "RCC.H"
#include <stdint.h>
 
/**
* @brief Initialzes a GPIO pin according to the provided configuration
*
* Configures port clock, mode, pull, output type, speed, and alternate function 
* registers for the specified pin. Enables the GPIO port clock via AHB1ENR
*
* @param cfg Pointer to a fully populated GPIO_Config_t struct
*
* @note Must be called before any other GPIO operation on the pin 
*      Alternate function is only configure when mode is GPIO_MODE_AF
*
* @retval None
*/
void GPIO_init(const GPIO_Config_t *cfg) {
    if(!cfg)
        return;

    SET_BIT(RCC->AHB1ENR, cfg->clock_bit);

    uint8_t pin = cfg->pin_num;
    cfg->port->MODER &= ~(3U << (pin * 2)); /* clear mode bits - 2 bits wide per pin */ // 
    cfg->port->MODER |= (cfg->mode << (pin * 2)); /* bit width 0b1 == 1U , 0b11 == 3U , 0b111 == 7U, 0b1111 == 0xFU */

    cfg->port->PUPDR &= ~(3U << (pin * 2)); /* clear pull bits */
    cfg->port->PUPDR |= (cfg->pull << (pin * 2));

    if(cfg->mode == GPIO_MODE_AF) {
        uint8_t afr_idx = cfg->pin_num / 8; /* pins 0-7 -> AFR[0] (AFRL), pins 8-15 -> AFR[1] (AFRH) */
        uint8_t afr_shift = (cfg->pin_num % 8) * 4; /* 4 bits per pin within the register */
                                                   
        cfg->port->AFR[afr_idx] &= ~(0xFU << afr_shift); /* clear AF bits for the pin */ 
        cfg->port->AFR[afr_idx] |= ((uint32_t)cfg->alternate_func << afr_shift); /* set AF bits */
    }

    if(cfg->type == GPIO_OTYPE_OPENDRAIN) { /* set output type*/
        cfg->port->OTYPER |= (1U << pin);
    } else {
        cfg->port->OTYPER &= ~(1U << pin);
    }

    cfg->port->OSPEEDR &= ~(3U << (pin * 2)); /* set output speed */
    cfg->port->OSPEEDR |= ((uint32_t)cfg->speed << (pin * 2));
}

/**
* @brief Sets a GPIO output pin high or low
*
* @param cfg Pointer to GPIO_Config_t struct for the target pin
* @param state GPIO_STATE_LOW (0) to set pin low, GPIO_STATE_HIGH (1) to set pin high
*
* @retval None
*/
void GPIO_write(GPIO_Config_t *cfg, GPIO_State_t state) {
    if(!cfg)
        return;
    
    if(state)
        GPIO_SET(cfg->port, cfg->pin_mask);
    else    
        GPIO_CLR(cfg->port, cfg->pin_mask);
}

/**
* @brief Toggles the current state of a GPIO output pin
*
* Reads the current ODR state and sets or clears the pin accordingly
*
* @param cfg Pointer to GPIO_Config_t struct for the target pin
*
* @retval None
*/
void GPIO_toggle(GPIO_Config_t *cfg) {
    if(!cfg)
        return;
    
    if(cfg->port->ODR & cfg->pin_mask) /* if pin is currently high, set low */
        GPIO_CLR(cfg->port, cfg->pin_mask);
    else 
        GPIO_SET(cfg->port, cfg->pin_mask);
}

/**
* @brief Reads the current state of a GPIO pin from the IDR register
*
* @param cfg Pointer to a GPIO_Config_t struct for the target pin
*
* @retval 1 if pin is high
* @retval 0 if pin is LOW or if cfg is NULL
*/
uint8_t GPIO_read(GPIO_Config_t *cfg) {
    if(!cfg)
        return 0;

    return(cfg->port->IDR & cfg->pin_mask) ? 1U : 0U;
}
