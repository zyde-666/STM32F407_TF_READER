#include "bsp_sd.h"

#include "app_config.h"

#if APP_SD_USE_SDIO

static SD_HandleTypeDef hsd;
static HAL_SD_CardInfoTypeDef card_info;
static uint8_t sd_initialized;

static bsp_sd_status_t wait_ready(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) < timeout_ms) {
        if (HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER) {
            return BSP_SD_OK;
        }
    }
    return BSP_SD_TIMEOUT;
}

uint8_t BSP_SD_IsDetected(void)
{
#if APP_SD_DETECT_ENABLE
    return HAL_GPIO_ReadPin(APP_SD_DETECT_GPIO_PORT, APP_SD_DETECT_PIN) ==
           APP_SD_DETECT_ACTIVE_STATE;
#else
    return 1U;
#endif
}

bsp_sd_status_t BSP_SD_Init(void)
{
    if (!BSP_SD_IsDetected()) {
        sd_initialized = 0U;
        return BSP_SD_NO_CARD;
    }

    if (sd_initialized) {
        return BSP_SD_OK;
    }

    hsd.Instance = SDIO;
    hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv = APP_SDIO_TRANSFER_CLK_DIV;

    if (HAL_SD_Init(&hsd) != HAL_OK) {
        sd_initialized = 0U;
        return BSP_SD_ERROR;
    }

#if APP_SDIO_USE_4BIT
    if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK) {
        sd_initialized = 0U;
        return BSP_SD_ERROR;
    }
#else
    if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_1B) != HAL_OK) {
        sd_initialized = 0U;
        return BSP_SD_ERROR;
    }
#endif

    if (HAL_SD_GetCardInfo(&hsd, &card_info) != HAL_OK) {
        sd_initialized = 0U;
        return BSP_SD_ERROR;
    }

    sd_initialized = 1U;
    return wait_ready(5000U);
}

bsp_sd_status_t BSP_SD_DeInit(void)
{
    sd_initialized = 0U;
    return (HAL_SD_DeInit(&hsd) == HAL_OK) ? BSP_SD_OK : BSP_SD_ERROR;
}

bsp_sd_status_t BSP_SD_ReadBlocks(uint8_t *data, uint32_t sector, uint32_t count)
{
    if (data == NULL || count == 0U) {
        return BSP_SD_PARAM_ERROR;
    }

    if (!sd_initialized && BSP_SD_Init() != BSP_SD_OK) {
        return BSP_SD_ERROR;
    }

    if (HAL_SD_ReadBlocks(&hsd, data, sector, count, 5000U) != HAL_OK) {
        return BSP_SD_ERROR;
    }

    return wait_ready(5000U);
}

bsp_sd_status_t BSP_SD_WriteBlocks(const uint8_t *data, uint32_t sector, uint32_t count)
{
    if (data == NULL || count == 0U) {
        return BSP_SD_PARAM_ERROR;
    }

    if (!sd_initialized && BSP_SD_Init() != BSP_SD_OK) {
        return BSP_SD_ERROR;
    }

    if (HAL_SD_WriteBlocks(&hsd, (uint8_t *)data, sector, count, 5000U) != HAL_OK) {
        return BSP_SD_ERROR;
    }

    return wait_ready(5000U);
}

bsp_sd_status_t BSP_SD_GetInfo(bsp_sd_info_t *info)
{
    if (info == NULL) {
        return BSP_SD_PARAM_ERROR;
    }

    if (!sd_initialized && BSP_SD_Init() != BSP_SD_OK) {
        return BSP_SD_ERROR;
    }

    info->sector_count = card_info.LogBlockNbr;
    info->sector_size = card_info.LogBlockSize;
    info->erase_block_size = 1U;
    return BSP_SD_OK;
}

uint8_t BSP_SD_IsReady(void)
{
    return sd_initialized &&
           (HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER);
}

void HAL_SD_MspInit(SD_HandleTypeDef *sd_handle)
{
    if (sd_handle->Instance == SDIO) {
        GPIO_InitTypeDef gpio = {0};

        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();
        __HAL_RCC_SDIO_CLK_ENABLE();

        gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_PULLUP;
        gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio.Alternate = GPIO_AF12_SDIO;
        HAL_GPIO_Init(GPIOC, &gpio);

        gpio.Pin = GPIO_PIN_2;
        HAL_GPIO_Init(GPIOD, &gpio);
    }
}

void HAL_SD_MspDeInit(SD_HandleTypeDef *sd_handle)
{
    if (sd_handle->Instance == SDIO) {
        __HAL_RCC_SDIO_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
        HAL_GPIO_DeInit(GPIOD, GPIO_PIN_2);
    }
}

#endif
