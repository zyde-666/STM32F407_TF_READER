#include "main.h"

void HAL_MspInit(void)
{
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        GPIO_InitTypeDef gpio = {0};

        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        gpio.Pin = GPIO_PIN_9 | GPIO_PIN_10;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_PULLUP;
        gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio.Alternate = GPIO_AF7_USART1;
        HAL_GPIO_Init(GPIOA, &gpio);
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1) {
        __HAL_RCC_USART1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9 | GPIO_PIN_10);
    }
}

void HAL_PCD_MspInit(PCD_HandleTypeDef *hpcd)
{
    if (hpcd->Instance == USB_OTG_FS) {
        GPIO_InitTypeDef gpio = {0};

        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_USB_OTG_FS_CLK_ENABLE();

        gpio.Pin = GPIO_PIN_11 | GPIO_PIN_12;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_NOPULL;
        gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio.Alternate = GPIO_AF10_OTG_FS;
        HAL_GPIO_Init(GPIOA, &gpio);

        HAL_NVIC_SetPriority(OTG_FS_IRQn, 5U, 0U);
        HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    }
}

void HAL_PCD_MspDeInit(PCD_HandleTypeDef *hpcd)
{
    if (hpcd->Instance == USB_OTG_FS) {
        __HAL_RCC_USB_OTG_FS_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);
        HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
    }
}
