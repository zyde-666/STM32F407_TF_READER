#include "diskio.h"

#include "bsp_sd.h"

#define SD_DRIVE 0U

static DSTATUS sd_status = STA_NOINIT;

DSTATUS disk_initialize(BYTE pdrv)
{
    if (pdrv != SD_DRIVE) {
        return STA_NOINIT;
    }

    if (BSP_SD_Init() == BSP_SD_OK) {
        sd_status = 0U;
    } else {
        sd_status = STA_NOINIT;
    }

    return sd_status;
}

DSTATUS disk_status(BYTE pdrv)
{
    if (pdrv != SD_DRIVE) {
        return STA_NOINIT;
    }

    if (!BSP_SD_IsDetected()) {
        sd_status = STA_NOINIT | STA_NODISK;
    } else if (BSP_SD_IsReady()) {
        sd_status = 0U;
    }

    return sd_status;
}

DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, UINT count)
{
    if (pdrv != SD_DRIVE || buff == 0 || count == 0U) {
        return RES_PARERR;
    }

    if (sd_status & STA_NOINIT) {
        return RES_NOTRDY;
    }

    return (BSP_SD_ReadBlocks(buff, sector, count) == BSP_SD_OK) ? RES_OK : RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE *buff, DWORD sector, UINT count)
{
    if (pdrv != SD_DRIVE || buff == 0 || count == 0U) {
        return RES_PARERR;
    }

    if (sd_status & STA_NOINIT) {
        return RES_NOTRDY;
    }

    return (BSP_SD_WriteBlocks(buff, sector, count) == BSP_SD_OK) ? RES_OK : RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
    bsp_sd_info_t info;

    if (pdrv != SD_DRIVE) {
        return RES_PARERR;
    }

    if (sd_status & STA_NOINIT) {
        return RES_NOTRDY;
    }

    switch (cmd) {
    case CTRL_SYNC:
        return BSP_SD_IsReady() ? RES_OK : RES_ERROR;

    case GET_SECTOR_COUNT:
        if (buff == 0 || BSP_SD_GetInfo(&info) != BSP_SD_OK) {
            return RES_ERROR;
        }
        *(DWORD *)buff = info.sector_count;
        return RES_OK;

    case GET_SECTOR_SIZE:
        if (buff == 0) {
            return RES_PARERR;
        }
        *(WORD *)buff = BSP_SD_BLOCK_SIZE;
        return RES_OK;

    case GET_BLOCK_SIZE:
        if (buff == 0 || BSP_SD_GetInfo(&info) != BSP_SD_OK) {
            return RES_ERROR;
        }
        *(DWORD *)buff = info.erase_block_size;
        return RES_OK;

    default:
        return RES_PARERR;
    }
}

DWORD get_fattime(void)
{
    return ((DWORD)(2026U - 1980U) << 25) |
           ((DWORD)7U << 21) |
           ((DWORD)22U << 16) |
           ((DWORD)12U << 11);
}
