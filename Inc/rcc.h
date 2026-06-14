/**
* @file rcc.h
* @brief RCC (Reset and Clock Control) register map for STM32F446RE
*
* Defines the RCC peripheral register map and base address pointer.
* Used by all peripheral drivers to enable clocks via AHB1ENR, APB1ENR,
* and APB2ENR before accessing any peripheral.
*
* Base address: 0x40023800
*
* @ref STM32F446RE Reference Manual RM0390 - Section 6 (RCC)
*/

#ifndef RCC_H
#define RCC_H

#include <stdint.h>

/** @brief RCC peripheral base address */
#define RCC ((RCC_RegMap_t*)0x40023800UL)

/**
s* @brief RCC peripheral register map.
*
* Mapped directly to hardware at base address 0x40023800.
* Reserved fields are included to maintain correct register offsets.
* Each register is 32 bits wide.
*/
typedef struct {
    volatile uint32_t CR;               /**< 0x00 - clock control */
    volatile uint32_t PLL_CFGR;         /**< 0x04 - PLL configuration */
    volatile uint32_t CFGR;             /**< 0x08 - clock configuration */
    volatile uint32_t CIR;              /**< 0x0C - clock interrupt */
    volatile uint32_t AHB1RSTR;         /**< 0x10 - AHB1 peripheral reset */
    volatile uint32_t AHB2RSTR;         /**< 0x14 - AHB2 peripheral reset */
    volatile uint32_t AHB3RSTR;         /**< 0x18 - AHB3 peripheral reset */
    volatile uint32_t RESERVED1[1];     /**< 0x1C - reserved */
    volatile uint32_t APB1RSTR;         /**< 0x20 - APB1 peripheral reset(I2C, UART, SPI) */
    volatile uint32_t APB2RSTR;         /**< 0x24 - APB2 peripheral reset(USART1, SPI1) */
    volatile uint32_t RESERVED2[1];     /**< 0x28 - reserved */
    volatile uint32_t RESERVED3[1];     /**< 0x2C - reserved */
    volatile uint32_t AHB1ENR;          /**< 0x30 - AHB1 clock enable (GPIOA-K, DMA, USB) */
    volatile uint32_t AHB2ENR;          /**< 0x34 - AHB2 clock enable (USB OTG) */
    volatile uint32_t AHB3ENR;          /**< 0x38 - AHB3 clock enable (FMC) */
    volatile uint32_t RESERVED4[1];     /**< 0x3C - reserved */
    volatile uint32_t APB1ENR;          /**< 0x40 - APB1 clock enable (I2C1, USART2, TIM2-7) */
    volatile uint32_t APB2ENR;          /**< 0x44 - APB2 clock enable (USART1, SPI1, TIM1) */
    volatile uint32_t RESERVED5[1];     /**< 0x48 - reserved */
    volatile uint32_t RESERVED6[1];     /**< 0x4C - reserved */
    volatile uint32_t AHB1LPENR;        /**< 0x50 - AHB1 low power clock enable */
    volatile uint32_t AHB2LPENR;        /**< 0x54 - AHB2 low power clock enable */
    volatile uint32_t AHB3LPENR;        /**< 0x58 - AHB3 low power clock enable */
    volatile uint32_t RESERVED7[1];     /**< 0x5C - reserved */
    volatile uint32_t APB1LPENR;        /**< 0x60 - APB1 low power clock enable (I2C, UART, SPI) */
    volatile uint32_t APB2LPENR;        /**< 0x64 - APB2 low power clock enable (USART1, SPI1, TIM1) */
    volatile uint32_t RESERVED8[1];     /**< 0x68 - reserved */
    volatile uint32_t RESERVED9[1];     /**< 0x6C - reserved */
    volatile uint32_t BDCR;             /**< 0x70 - backup domain control (RTC, LSE) */
    volatile uint32_t CSR;              /**< 0x74 - control/status (LSI, reset flags) */
    volatile uint32_t RESERVED10[1];    /**< 0x78 - reserved */
    volatile uint32_t RESERVED11[1];    /**< 0x7C - reserved */
    volatile uint32_t SSCGR;            /**< 0x80 - spread spectrum clock generation */
    volatile uint32_t PLLI2_SCFGR;      /**< 0x84 - PLLI2S configuration (I2S clock) */
    volatile uint32_t PLL_SAI_CFGR;     /**< 0x88 - PLLSAI configuration (SAI, LTDC clock) */
    volatile uint32_t DCKCFGR;          /**< 0x8C - dedicated clock configuration */
    volatile uint32_t CKGATENR;         /**< 0x90 - clock gating enable */
    volatile uint32_t DCKCFGR2;         /**< 0x94 - dedicated clock configuration 2 */
} RCC_RegMap_t;

#endif /* RCC_H */


