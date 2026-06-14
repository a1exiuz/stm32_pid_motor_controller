/**
* @file gpio.h
* @brief GPIO driver for STM32F446RE
*
* Register map, configuration types, and control functions for
* STM32F446RE GPIO peripherals. Supports input, output, alternate
* function, and analog modes with configurable pull, speed, and output type.
*    
* BASE addresses:
*        GPIOA: 0x40020000 - GPIOB: 0x40020400 - GPIOC: 0x40020800
*    
* @ref STM32F446RE Reference Manual RM0390
*/
#ifndef GPIO_H
#define GPIO_H

#include "RCC.H"
#include <stdint.h>

/** @brief GPIOA peripheral base address */
#define GPIOA ((GPIO_RegMap_t*)0x40020000UL)

/** @brief GPIOB peripheral base address */
#define GPIOB ((GPIO_RegMap_t*)0x40020400UL)

/** @brief GPIOC peripheral base address */
#define GPIOC ((GPIO_RegMap_t*)0x40020800UL)

/**
* @defgroup BIT_MACROS Bit Manipulation Macros
* @brief    Register-level bit manipulation utilities
* @{
*/
#define SET_BIT(reg, bit) ((reg) |= (1U << (bit)))  /**< Set a single bit */
#define CLR_BIT(reg, bit) ((reg) &= ~(1U << (bit))) /**< Clear a single bit */
#define TGL_BIT(reg, bit) ((reg) ^= (1U << (bit))) /**< Toggle a single bit */
#define READ_BIT(reg, bit) (((reg) >> (bit)) & 1U) /**< Read a single bit */
/** @} */

/**
* @defgroup GPIO_MACROS GPIO Set/clear Macros
* @brief    Atomic pin set and clear via BSRR - no read-modify-write needed
* @{
*/
#define GPIO_SET(port, mask) ((port)->BSRR = (mask))         /**< Set pin(s) high */
#define GPIO_CLR(port, mask) ((port)->BSRR = (mask) << 16U)  /**< Set pin(s) low */
/** @} */

/**
* @brief GPIO peripheral register map
*
* Mapped directly to hardware at the GPIO base address
* Each register is 32 bits wide
*/
typedef struct {
    volatile uint32_t MODER;        /** <0x00 - pin mode (input, output, AF, analog) */
    volatile uint32_t OTYPER;       /**< 0x04 - output type (push-pull or open-drain) */
    volatile uint32_t OSPEEDR;      /**< 0x08 - output spped (low, medium, fast, high) */
    volatile uint32_t PUPDR;        /**< 0x0C - pull-up/pull-down resistor config */
    volatile uint32_t IDR;          /**< 0x10 - input data register (read only) */
    volatile uint32_t ODR;          /**< 0x14 - output data register */
    volatile uint32_t BSRR;         /**< 0x18 - bit set/reset (atomic set/clear of ODR) */
    volatile uint32_t LCKR;         /**< 0x1C - lock register (locks pin config) */
    volatile uint32_t AFR[2];       /**< 0x20-0x24 - alternate function (AFR[0]=pins 0-7), AFR[1]=pins 8-15) */
} GPIO_RegMap_t;

/**
* @brief GPIO pin mode (MODER register, 2 bits per pin)
*/
typedef enum {
    GPIO_MODE_INPUT = 0, /**< high impedance input */
    GPIO_MODE_OUTPUT,    /**< general purpose output */
    GPIO_MODE_AF,        /**< alternate function (I2C, UART, SPI etc) */
    GPIO_MODE_ANALOG     /**<  analog mode (ADC/DAC) */
} GPIO_Mode_t;

/**
* @brief GPIO pull resistor configuration (PUPDR register, 2 bits per pin)
*/
typedef enum {
    GPIO_NOPULL,         /**< floating - no internal resistor */
    GPIO_PULLUP,         /**< internal pull-up to VDD */
    GPIO_PULLDOWN        /**< internal pull-down to GND */
} GPIO_Pull_t;

/**
* @brief GPIO pin output speed (OSPEEDR register, 2 bits per pin)
*/
typedef enum {
    GPIO_SPEED_LOW,     /**< 2MHz */
    GPIO_SPEED_MEDIEUM, /**< 25MHz */
    GPIO_SPEED_FAST,    /**< 50MHz */
    GPIO_SPEED_HIGH     /**< 100MHz - recommended  for I2C, SPI, UART */
} GPIO_Speed_t;

/**
* @brief GPIO pin state for GPIO_write()
*/
typedef enum {
    GPIO_LOW,  /**< Drive pin low */
    GPIO_HIGH  /**< Drive pin high */
} GPIO_State_t;

/**
* @brief GPIO output type (OTYPER register, 1 bit per pin)
*/
typedef enum {
    GPIO_OTYPE_PUSHPULL, /**< Drives high and low - default for most pins */
    GPIO_OTYPE_OPENDRAIN /**< Only drives low - requires external pull-up for I2C */
} GPIO_OType_t;

/**
* @brief GPIO pin configuration struct passed to GPIO_init()
*
* Fill all fields before calling GPIO_init()
* alternate_funct is only used when mode is GPIO_MODE_AF
*/
typedef struct {
    GPIO_RegMap_t *port;    /**< GPIO port (GPIOA, GPIOB, GPIOC) */
    uint8_t clock_bit;      /**< RCC AHBB1ENR for this port (0=A, 1=B, 2=C...) */
    uint32_t pin_mask;      /**< bitmask of pin (1U << pin_num) */
    uint32_t pin_num;       /**< pin number 0-15 */

    GPIO_Mode_t mode;       /**< input,output, AF, or analog */
    GPIO_Pull_t pull;       /**< pull-up, pull-down, or none */
    GPIO_OType_t type;      /**< push-pull or open drain */

    GPIO_Speed_t speed;     // output speed */
    uint8_t alternate_func; // AF number 0-15(only used when mode = GPIO_MODE_AF */
} GPIO_Config_t;

/**
* @brief Initializes a GPIO pin according to the provided configuration
*
* Configures port clock, mode, pull, output type, speed,
* and alternate function registers for the specified pin
* Enables the GPIO port clock
*
* @param cfg Pointer to a fully populated GPIO_Config_t struct
*
* @note Must be called before any other GPIO operation on the pin
*       Alternate function is only configured when mode is GPIO_MODE_AF
*
* @retval None
*/
void GPIO_init(const GPIO_Config_t *cfg);

/**
* @brief Sets a GPIO output pin high or low
*
* Uses the BSRR register for atomic operation - no read-modify-write
*
* @param cfg Pointer to GPIO_Config_t for the target pin
* @param state GPIO_LOW, or GPIO_HIGH
*
* @retval None
*/
void GPIO_write(GPIO_Config_t *cfg, GPIO_State_t state);

/** 
* @brief Toggles the current state of a GPIO output pin
* 
* @param cfg Pointer to GPIO_Config_t for the target pin
*
* @retval None
*/
void GPIO_toggle(GPIO_Config_t *cfg);

/**
* @brief Reads the current state of a GPIO pin from the IDR register
*
* @param cfg Pointer to GPIO_Config_t for the target pin
*
* @retval 1 if pin is HIGH
* @retval 0 if pin is LOW or cfg is NULL
*/
uint8_t GPIO_read(GPIO_Config_t *cfg);

#endif /* GPIO_H */
