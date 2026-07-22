#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include "stm32f4xx_hal.h"

/* Most STM32F407ZGT6 core boards use an 8 MHz HSE. If your board uses another
 * crystal, change HSE_VALUE in stm32f4xx_hal_conf.h. PLLM follows it by default. */
#ifndef APP_PLL_M
#define APP_PLL_M (HSE_VALUE / 1000000U)
#endif
#ifndef APP_PLL_N
#define APP_PLL_N 336U
#endif
#ifndef APP_PLL_P
#define APP_PLL_P RCC_PLLP_DIV2
#endif
#ifndef APP_PLL_Q
#define APP_PLL_Q 7U
#endif

#ifndef APP_UART_BAUDRATE
#define APP_UART_BAUDRATE 115200U
#endif

/* 1: SDIO 4-bit/1-bit mode, 0: SPI mode. */
#ifndef APP_SD_USE_SDIO
#define APP_SD_USE_SDIO 1
#endif

/* Standard STM32F407 SDIO pins:
 * PC8=D0 PC9=D1 PC10=D2 PC11=D3 PC12=CLK PD2=CMD */
#ifndef APP_SDIO_USE_4BIT
#define APP_SDIO_USE_4BIT 1
#endif
#ifndef APP_SDIO_TRANSFER_CLK_DIV
#define APP_SDIO_TRANSFER_CLK_DIV 4U
#endif

/* Many TF modules do not wire card-detect. Keep this disabled by default. */
#ifndef APP_SD_DETECT_ENABLE
#define APP_SD_DETECT_ENABLE 0
#endif
#define APP_SD_DETECT_GPIO_PORT GPIOC
#define APP_SD_DETECT_PIN GPIO_PIN_13
#define APP_SD_DETECT_ACTIVE_STATE GPIO_PIN_RESET

/* SPI fallback defaults: SPI1 on PA5/PA6/PA7, CS on PA4. */
#define APP_SD_SPI_INSTANCE SPI1
#define APP_SD_SPI_GPIO_AF GPIO_AF5_SPI1
#define APP_SD_SPI_SCK_GPIO_PORT GPIOA
#define APP_SD_SPI_SCK_PIN GPIO_PIN_5
#define APP_SD_SPI_MISO_GPIO_PORT GPIOA
#define APP_SD_SPI_MISO_PIN GPIO_PIN_6
#define APP_SD_SPI_MOSI_GPIO_PORT GPIOA
#define APP_SD_SPI_MOSI_PIN GPIO_PIN_7
#define APP_SD_SPI_CS_GPIO_PORT GPIOA
#define APP_SD_SPI_CS_PIN GPIO_PIN_4
#define APP_SD_SPI_LOW_SPEED_PRESCALER SPI_BAUDRATEPRESCALER_256
#define APP_SD_SPI_HIGH_SPEED_PRESCALER SPI_BAUDRATEPRESCALER_4

/* The project is intentionally strict: it mounts FAT, then requires FAT32. */
#ifndef APP_REQUIRE_FAT32
#define APP_REQUIRE_FAT32 1
#endif
#ifndef APP_WRITE_TEST_FILE
#define APP_WRITE_TEST_FILE 1
#endif

/* USB Mass Storage exposes the TF card to a PC as a standard card reader. */
#ifndef APP_ENABLE_USB_MSC
#define APP_ENABLE_USB_MSC 1
#endif
#ifndef APP_USB_MSC_READONLY
#define APP_USB_MSC_READONLY 0
#endif

#endif
