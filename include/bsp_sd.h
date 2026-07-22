#ifndef BSP_SD_H
#define BSP_SD_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

#define BSP_SD_BLOCK_SIZE 512U

typedef enum {
    BSP_SD_OK = 0,
    BSP_SD_ERROR = -1,
    BSP_SD_NO_CARD = -2,
    BSP_SD_TIMEOUT = -3,
    BSP_SD_PARAM_ERROR = -4,
} bsp_sd_status_t;

typedef struct {
    uint32_t sector_count;
    uint32_t sector_size;
    uint32_t erase_block_size;
} bsp_sd_info_t;

bsp_sd_status_t BSP_SD_Init(void);
bsp_sd_status_t BSP_SD_DeInit(void);
bsp_sd_status_t BSP_SD_ReadBlocks(uint8_t *data, uint32_t sector, uint32_t count);
bsp_sd_status_t BSP_SD_WriteBlocks(const uint8_t *data, uint32_t sector, uint32_t count);
bsp_sd_status_t BSP_SD_GetInfo(bsp_sd_info_t *info);
uint8_t BSP_SD_IsDetected(void);
uint8_t BSP_SD_IsReady(void);

#endif
