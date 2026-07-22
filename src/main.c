#include "main.h"

#include <stdio.h>

#include "app_config.h"
#include "bsp_sd.h"
#include "fatfs_app.h"
#include "usb_device.h"

UART_HandleTypeDef huart1;

static void MX_GPIO_Init(void);
static void MX_USART1_UART_Init(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();

    printf("\r\nSTM32F407ZG TF reader start\r\n");
    printf("Clock: %lu Hz, UART: %lu\r\n", HAL_RCC_GetHCLKFreq(), (unsigned long)APP_UART_BAUDRATE);

#if APP_ENABLE_USB_MSC
    while (BSP_SD_Init() != BSP_SD_OK) {
        printf("Waiting for TF card...\r\n");
        HAL_Delay(1000);
    }

    int self_test = FatFsApp_Run();
    if (self_test == 0) {
        printf("FAT32 self-check passed. Starting USB MSC reader...\r\n");
    } else {
        printf("FAT32 self-check failed (%d). USB MSC still starts so PC can inspect/format the card.\r\n",
               self_test);
    }

    MX_USB_DEVICE_Init();
    printf("USB MSC ready. Connect PA11/PA12 USB FS to PC.\r\n");

    while (1) {
        HAL_Delay(1000);
    }
#else
    for (;;) {
        int result = FatFsApp_Run();
        if (result == 0) {
            printf("TF card FAT32 check passed. Reader is ready.\r\n");
            while (1) {
                HAL_Delay(1000);
            }
        }

        printf("TF card check failed (%d). Retry in 2 seconds...\r\n", result);
        HAL_Delay(2000);
    }
#endif
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef osc = {0};
    RCC_ClkInitTypeDef clk = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    osc.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    osc.HSEState = RCC_HSE_ON;
    osc.PLL.PLLState = RCC_PLL_ON;
    osc.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    osc.PLL.PLLM = APP_PLL_M;
    osc.PLL.PLLN = APP_PLL_N;
    osc.PLL.PLLP = APP_PLL_P;
    osc.PLL.PLLQ = APP_PLL_Q;
    if (HAL_RCC_OscConfig(&osc) != HAL_OK) {
        Error_Handler();
    }

    clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                    RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
    clk.APB1CLKDivider = RCC_HCLK_DIV4;
    clk.APB2CLKDivider = RCC_HCLK_DIV2;
    if (HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_5) != HAL_OK) {
        Error_Handler();
    }

    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000U);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    HAL_NVIC_SetPriority(SysTick_IRQn, TICK_INT_PRIORITY, 0U);
}

static void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = APP_UART_BAUDRATE;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        Error_Handler();
    }
}

static void MX_GPIO_Init(void)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

#if APP_SD_DETECT_ENABLE
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = APP_SD_DETECT_PIN;
    gpio.Mode = GPIO_MODE_INPUT;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(APP_SD_DETECT_GPIO_PORT, &gpio);
#endif
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    (void)file;
    (void)line;
    Error_Handler();
}
#endif
