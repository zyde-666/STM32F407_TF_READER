#include "usbd_storage_if.h"

#include "app_config.h"
#include "bsp_sd.h"

#define STORAGE_LUN_NBR 1U

static int8_t STORAGE_Init_FS(uint8_t lun);
static int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady_FS(uint8_t lun);
static int8_t STORAGE_IsWriteProtected_FS(uint8_t lun);
static int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun_FS(void);

static int8_t STORAGE_Inquirydata_FS[] = {
    0x00, 0x80, 0x02, 0x02,
    (STANDARD_INQUIRY_DATA_LEN - 5U),
    0x00, 0x00, 0x00,
    'S', 'T', 'M', '3', '2', ' ', ' ', ' ',
    'T', 'F', ' ', 'R', 'e', 'a', 'd', 'e',
    'r', ' ', 'F', 'A', 'T', '3', '2', ' ',
    '1', '.', '0', '0',
};

USBD_StorageTypeDef USBD_Storage_Interface_fops_FS = {
    STORAGE_Init_FS,
    STORAGE_GetCapacity_FS,
    STORAGE_IsReady_FS,
    STORAGE_IsWriteProtected_FS,
    STORAGE_Read_FS,
    STORAGE_Write_FS,
    STORAGE_GetMaxLun_FS,
    STORAGE_Inquirydata_FS,
};

static int8_t STORAGE_Init_FS(uint8_t lun)
{
    (void)lun;
    return (BSP_SD_Init() == BSP_SD_OK) ? 0 : -1;
}

static int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
    bsp_sd_info_t info;
    (void)lun;

    if (block_num == NULL || block_size == NULL || BSP_SD_GetInfo(&info) != BSP_SD_OK) {
        return -1;
    }

    *block_num = info.sector_count;
    *block_size = (uint16_t)info.sector_size;
    return 0;
}

static int8_t STORAGE_IsReady_FS(uint8_t lun)
{
    (void)lun;
    return BSP_SD_IsReady() ? 0 : -1;
}

static int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
    (void)lun;
#if APP_USB_MSC_READONLY
    return -1;
#else
    return 0;
#endif
}

static int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    (void)lun;
    return (BSP_SD_ReadBlocks(buf, blk_addr, blk_len) == BSP_SD_OK) ? 0 : -1;
}

static int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
    (void)lun;
#if APP_USB_MSC_READONLY
    (void)buf;
    (void)blk_addr;
    (void)blk_len;
    return -1;
#else
    return (BSP_SD_WriteBlocks(buf, blk_addr, blk_len) == BSP_SD_OK) ? 0 : -1;
#endif
}

static int8_t STORAGE_GetMaxLun_FS(void)
{
    return (STORAGE_LUN_NBR - 1U);
}
