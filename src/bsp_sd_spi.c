#include "bsp_sd.h"

#include "app_config.h"

#if !APP_SD_USE_SDIO

#define CMD0 (0U)
#define CMD1 (1U)
#define CMD8 (8U)
#define CMD9 (9U)
#define CMD12 (12U)
#define CMD16 (16U)
#define CMD17 (17U)
#define CMD24 (24U)
#define CMD55 (55U)
#define CMD58 (58U)
#define ACMD41 (0x80U + 41U)

#define CT_MMC 0x01U
#define CT_SD1 0x02U
#define CT_SD2 0x04U
#define CT_BLOCK 0x08U

static SPI_HandleTypeDef hspi_sd;
static uint8_t card_type;
static uint32_t sector_count;
static uint8_t sd_initialized;

static void spi_set_speed(uint32_t prescaler)
{
    hspi_sd.Instance = APP_SD_SPI_INSTANCE;
    hspi_sd.Init.Mode = SPI_MODE_MASTER;
    hspi_sd.Init.Direction = SPI_DIRECTION_2LINES;
    hspi_sd.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi_sd.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi_sd.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi_sd.Init.NSS = SPI_NSS_SOFT;
    hspi_sd.Init.BaudRatePrescaler = prescaler;
    hspi_sd.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi_sd.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi_sd.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi_sd.Init.CRCPolynomial = 7U;
    (void)HAL_SPI_Init(&hspi_sd);
}

static uint8_t spi_xfer(uint8_t data)
{
    uint8_t rx = 0xFFU;
    (void)HAL_SPI_TransmitReceive(&hspi_sd, &data, &rx, 1U, HAL_MAX_DELAY);
    return rx;
}

static void cs_high(void)
{
    HAL_GPIO_WritePin(APP_SD_SPI_CS_GPIO_PORT, APP_SD_SPI_CS_PIN, GPIO_PIN_SET);
    (void)spi_xfer(0xFFU);
}

static void cs_low(void)
{
    HAL_GPIO_WritePin(APP_SD_SPI_CS_GPIO_PORT, APP_SD_SPI_CS_PIN, GPIO_PIN_RESET);
}

static uint8_t wait_ready(uint32_t timeout_ms)
{
    uint32_t start = HAL_GetTick();
    uint8_t value;

    do {
        value = spi_xfer(0xFFU);
        if (value == 0xFFU) {
            return 1U;
        }
    } while ((HAL_GetTick() - start) < timeout_ms);

    return 0U;
}

static void deselect_card(void)
{
    cs_high();
}

static uint8_t select_card(void)
{
    cs_low();
    if (wait_ready(500U)) {
        return 1U;
    }
    deselect_card();
    return 0U;
}

static uint8_t send_cmd(uint8_t cmd, uint32_t arg)
{
    uint8_t crc = 0x01U;
    uint8_t response;

    if (cmd & 0x80U) {
        cmd &= 0x7FU;
        response = send_cmd(CMD55, 0U);
        if (response > 1U) {
            return response;
        }
    }

    deselect_card();
    if (!select_card()) {
        return 0xFFU;
    }

    spi_xfer(0x40U | cmd);
    spi_xfer((uint8_t)(arg >> 24));
    spi_xfer((uint8_t)(arg >> 16));
    spi_xfer((uint8_t)(arg >> 8));
    spi_xfer((uint8_t)arg);

    if (cmd == CMD0) {
        crc = 0x95U;
    } else if (cmd == CMD8) {
        crc = 0x87U;
    }
    spi_xfer(crc);

    for (uint8_t retry = 10U; retry > 0U; retry--) {
        response = spi_xfer(0xFFU);
        if ((response & 0x80U) == 0U) {
            return response;
        }
    }

    return 0xFFU;
}

static uint8_t read_data_block(uint8_t *buffer, uint16_t length)
{
    uint8_t token;
    uint32_t start = HAL_GetTick();

    do {
        token = spi_xfer(0xFFU);
        if (token == 0xFEU) {
            for (uint16_t i = 0U; i < length; i++) {
                buffer[i] = spi_xfer(0xFFU);
            }
            spi_xfer(0xFFU);
            spi_xfer(0xFFU);
            return 1U;
        }
    } while ((HAL_GetTick() - start) < 500U);

    return 0U;
}

static uint8_t write_data_block(const uint8_t *buffer, uint8_t token)
{
    uint8_t response;

    if (!wait_ready(500U)) {
        return 0U;
    }

    spi_xfer(token);
    if (token == 0xFDU) {
        return 1U;
    }

    for (uint16_t i = 0U; i < BSP_SD_BLOCK_SIZE; i++) {
        spi_xfer(buffer[i]);
    }
    spi_xfer(0xFFU);
    spi_xfer(0xFFU);

    response = spi_xfer(0xFFU);
    return ((response & 0x1FU) == 0x05U);
}

static uint8_t read_csd(uint8_t *csd)
{
    uint8_t ok = 0U;

    if (send_cmd(CMD9, 0U) == 0U) {
        ok = read_data_block(csd, 16U);
    }
    deselect_card();
    return ok;
}

static uint32_t parse_sector_count(const uint8_t *csd)
{
    uint32_t csize;
    uint8_t n;

    if ((csd[0] & 0xC0U) == 0x40U) {
        csize = ((uint32_t)(csd[7] & 0x3FU) << 16) |
                ((uint32_t)csd[8] << 8) |
                csd[9];
        return (csize + 1U) << 10;
    }

    n = (uint8_t)((csd[5] & 0x0FU) + ((csd[10] & 0x80U) >> 7) +
                 ((csd[9] & 0x03U) << 1) + 2U);
    csize = ((uint32_t)(csd[8] >> 6) |
             ((uint32_t)csd[7] << 2) |
             ((uint32_t)(csd[6] & 0x03U) << 10)) + 1U;
    return csize << (n - 9U);
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
    uint8_t response;
    uint8_t ocr[4];
    uint8_t csd[16];
    uint32_t start;

    if (!BSP_SD_IsDetected()) {
        sd_initialized = 0U;
        return BSP_SD_NO_CARD;
    }

    if (sd_initialized) {
        return BSP_SD_OK;
    }

    sector_count = 0U;
    card_type = 0U;

    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef cs_gpio = {0};
    cs_gpio.Pin = APP_SD_SPI_CS_PIN;
    cs_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    cs_gpio.Pull = GPIO_PULLUP;
    cs_gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(APP_SD_SPI_CS_GPIO_PORT, &cs_gpio);
    cs_high();

    spi_set_speed(APP_SD_SPI_LOW_SPEED_PRESCALER);

    for (uint8_t i = 0U; i < 10U; i++) {
        spi_xfer(0xFFU);
    }

    if (send_cmd(CMD0, 0U) != 1U) {
        deselect_card();
        return BSP_SD_ERROR;
    }

    response = send_cmd(CMD8, 0x1AAU);
    if (response == 1U) {
        for (uint8_t i = 0U; i < 4U; i++) {
            ocr[i] = spi_xfer(0xFFU);
        }
        if (ocr[2] != 0x01U || ocr[3] != 0xAAU) {
            deselect_card();
            return BSP_SD_ERROR;
        }

        start = HAL_GetTick();
        do {
            response = send_cmd(ACMD41, 0x40000000U);
        } while (response != 0U && (HAL_GetTick() - start) < 1000U);

        if (response == 0U && send_cmd(CMD58, 0U) == 0U) {
            for (uint8_t i = 0U; i < 4U; i++) {
                ocr[i] = spi_xfer(0xFFU);
            }
            card_type = CT_SD2 | ((ocr[0] & 0x40U) ? CT_BLOCK : 0U);
        }
    } else {
        if (send_cmd(ACMD41, 0U) <= 1U) {
            card_type = CT_SD1;
            start = HAL_GetTick();
            do {
                response = send_cmd(ACMD41, 0U);
            } while (response != 0U && (HAL_GetTick() - start) < 1000U);
        } else {
            card_type = CT_MMC;
            start = HAL_GetTick();
            do {
                response = send_cmd(CMD1, 0U);
            } while (response != 0U && (HAL_GetTick() - start) < 1000U);
        }

        if (response != 0U || send_cmd(CMD16, BSP_SD_BLOCK_SIZE) != 0U) {
            card_type = 0U;
        }
    }

    deselect_card();

    if (card_type == 0U || !read_csd(csd)) {
        sd_initialized = 0U;
        return BSP_SD_ERROR;
    }

    sector_count = parse_sector_count(csd);
    spi_set_speed(APP_SD_SPI_HIGH_SPEED_PRESCALER);

    sd_initialized = 1U;
    return BSP_SD_OK;
}

bsp_sd_status_t BSP_SD_DeInit(void)
{
    sd_initialized = 0U;
    (void)HAL_SPI_DeInit(&hspi_sd);
    return BSP_SD_OK;
}

bsp_sd_status_t BSP_SD_ReadBlocks(uint8_t *data, uint32_t sector, uint32_t count)
{
    if (data == NULL || count == 0U) {
        return BSP_SD_PARAM_ERROR;
    }
    if (!sd_initialized && BSP_SD_Init() != BSP_SD_OK) {
        return BSP_SD_ERROR;
    }

    for (uint32_t i = 0U; i < count; i++) {
        uint32_t address = sector + i;
        if ((card_type & CT_BLOCK) == 0U) {
            address *= BSP_SD_BLOCK_SIZE;
        }
        if (send_cmd(CMD17, address) != 0U ||
            !read_data_block(data + (i * BSP_SD_BLOCK_SIZE), BSP_SD_BLOCK_SIZE)) {
            deselect_card();
            return BSP_SD_ERROR;
        }
        deselect_card();
    }

    return BSP_SD_OK;
}

bsp_sd_status_t BSP_SD_WriteBlocks(const uint8_t *data, uint32_t sector, uint32_t count)
{
    if (data == NULL || count == 0U) {
        return BSP_SD_PARAM_ERROR;
    }
    if (!sd_initialized && BSP_SD_Init() != BSP_SD_OK) {
        return BSP_SD_ERROR;
    }

    for (uint32_t i = 0U; i < count; i++) {
        uint32_t address = sector + i;
        if ((card_type & CT_BLOCK) == 0U) {
            address *= BSP_SD_BLOCK_SIZE;
        }
        if (send_cmd(CMD24, address) != 0U ||
            !write_data_block(data + (i * BSP_SD_BLOCK_SIZE), 0xFEU)) {
            deselect_card();
            return BSP_SD_ERROR;
        }
        deselect_card();
    }

    return BSP_SD_OK;
}

bsp_sd_status_t BSP_SD_GetInfo(bsp_sd_info_t *info)
{
    if (info == NULL) {
        return BSP_SD_PARAM_ERROR;
    }
    if (!sd_initialized && BSP_SD_Init() != BSP_SD_OK) {
        return BSP_SD_ERROR;
    }

    info->sector_count = sector_count;
    info->sector_size = BSP_SD_BLOCK_SIZE;
    info->erase_block_size = 1U;
    return BSP_SD_OK;
}

uint8_t BSP_SD_IsReady(void)
{
    return sd_initialized;
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == APP_SD_SPI_INSTANCE) {
        GPIO_InitTypeDef gpio = {0};

        if (hspi->Instance == SPI1) {
            __HAL_RCC_SPI1_CLK_ENABLE();
        } else if (hspi->Instance == SPI2) {
            __HAL_RCC_SPI2_CLK_ENABLE();
        } else if (hspi->Instance == SPI3) {
            __HAL_RCC_SPI3_CLK_ENABLE();
        }

        __HAL_RCC_GPIOA_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        __HAL_RCC_GPIOD_CLK_ENABLE();

        gpio.Mode = GPIO_MODE_AF_PP;
        gpio.Pull = GPIO_PULLUP;
        gpio.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
        gpio.Alternate = APP_SD_SPI_GPIO_AF;

        gpio.Pin = APP_SD_SPI_SCK_PIN;
        HAL_GPIO_Init(APP_SD_SPI_SCK_GPIO_PORT, &gpio);
        gpio.Pin = APP_SD_SPI_MISO_PIN;
        HAL_GPIO_Init(APP_SD_SPI_MISO_GPIO_PORT, &gpio);
        gpio.Pin = APP_SD_SPI_MOSI_PIN;
        HAL_GPIO_Init(APP_SD_SPI_MOSI_GPIO_PORT, &gpio);
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == APP_SD_SPI_INSTANCE) {
        if (hspi->Instance == SPI1) {
            __HAL_RCC_SPI1_CLK_DISABLE();
        } else if (hspi->Instance == SPI2) {
            __HAL_RCC_SPI2_CLK_DISABLE();
        } else if (hspi->Instance == SPI3) {
            __HAL_RCC_SPI3_CLK_DISABLE();
        }

        HAL_GPIO_DeInit(APP_SD_SPI_SCK_GPIO_PORT, APP_SD_SPI_SCK_PIN);
        HAL_GPIO_DeInit(APP_SD_SPI_MISO_GPIO_PORT, APP_SD_SPI_MISO_PIN);
        HAL_GPIO_DeInit(APP_SD_SPI_MOSI_GPIO_PORT, APP_SD_SPI_MOSI_PIN);
        HAL_GPIO_DeInit(APP_SD_SPI_CS_GPIO_PORT, APP_SD_SPI_CS_PIN);
    }
}

#endif
