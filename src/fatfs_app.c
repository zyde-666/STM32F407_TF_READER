#include "fatfs_app.h"

#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "bsp_sd.h"
#include "ff.h"

static FATFS sd_fs;

static const char *fresult_name(FRESULT result)
{
    switch (result) {
    case FR_OK: return "FR_OK";
    case FR_DISK_ERR: return "FR_DISK_ERR";
    case FR_INT_ERR: return "FR_INT_ERR";
    case FR_NOT_READY: return "FR_NOT_READY";
    case FR_NO_FILE: return "FR_NO_FILE";
    case FR_NO_PATH: return "FR_NO_PATH";
    case FR_INVALID_NAME: return "FR_INVALID_NAME";
    case FR_DENIED: return "FR_DENIED";
    case FR_EXIST: return "FR_EXIST";
    case FR_INVALID_OBJECT: return "FR_INVALID_OBJECT";
    case FR_WRITE_PROTECTED: return "FR_WRITE_PROTECTED";
    case FR_INVALID_DRIVE: return "FR_INVALID_DRIVE";
    case FR_NOT_ENABLED: return "FR_NOT_ENABLED";
    case FR_NO_FILESYSTEM: return "FR_NO_FILESYSTEM";
    case FR_MKFS_ABORTED: return "FR_MKFS_ABORTED";
    case FR_TIMEOUT: return "FR_TIMEOUT";
    case FR_LOCKED: return "FR_LOCKED";
    case FR_NOT_ENOUGH_CORE: return "FR_NOT_ENOUGH_CORE";
    case FR_TOO_MANY_OPEN_FILES: return "FR_TOO_MANY_OPEN_FILES";
    case FR_INVALID_PARAMETER: return "FR_INVALID_PARAMETER";
    default: return "FR_UNKNOWN";
    }
}

static const char *fs_type_name(BYTE fs_type)
{
    switch (fs_type) {
    case FS_FAT12: return "FAT12";
    case FS_FAT16: return "FAT16";
    case FS_FAT32: return "FAT32";
    case FS_EXFAT: return "exFAT";
    default: return "unknown";
    }
}

static int print_root_dir(void)
{
    DIR dir;
    FILINFO info;
    FRESULT fr;
    unsigned int shown = 0U;

    fr = f_opendir(&dir, "0:/");
    if (fr != FR_OK) {
        printf("f_opendir failed: %s\r\n", fresult_name(fr));
        return -1;
    }

    printf("Root directory:\r\n");
    for (;;) {
        fr = f_readdir(&dir, &info);
        if (fr != FR_OK || info.fname[0] == '\0') {
            break;
        }

        printf("  %c %10lu  %s\r\n",
               (info.fattrib & AM_DIR) ? 'D' : 'F',
               (unsigned long)info.fsize,
               info.fname);
        shown++;
        if (shown >= 16U) {
            printf("  ...\r\n");
            break;
        }
    }

    f_closedir(&dir);
    return (fr == FR_OK) ? 0 : -1;
}

static int write_read_test(void)
{
#if APP_WRITE_TEST_FILE
    static const char test_text[] =
        "STM32F407ZGT6 TF reader FAT32 write/read test OK\r\n";
    char readback[sizeof(test_text)] = {0};
    FIL file;
    UINT done = 0U;
    FRESULT fr;

    fr = f_open(&file, "0:/STM32F4_TF_TEST.TXT", FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) {
        printf("f_open write failed: %s\r\n", fresult_name(fr));
        return -1;
    }

    fr = f_write(&file, test_text, (UINT)strlen(test_text), &done);
    if (fr != FR_OK || done != strlen(test_text)) {
        printf("f_write failed: %s, bytes=%u\r\n", fresult_name(fr), done);
        f_close(&file);
        return -1;
    }

    fr = f_close(&file);
    if (fr != FR_OK) {
        printf("f_close write failed: %s\r\n", fresult_name(fr));
        return -1;
    }

    fr = f_open(&file, "0:/STM32F4_TF_TEST.TXT", FA_READ);
    if (fr != FR_OK) {
        printf("f_open read failed: %s\r\n", fresult_name(fr));
        return -1;
    }

    fr = f_read(&file, readback, sizeof(readback) - 1U, &done);
    f_close(&file);
    if (fr != FR_OK) {
        printf("f_read failed: %s\r\n", fresult_name(fr));
        return -1;
    }

    if (strncmp(readback, test_text, strlen(test_text)) != 0) {
        printf("Readback mismatch\r\n");
        return -1;
    }

    printf("Write/read test passed: 0:/STM32F4_TF_TEST.TXT\r\n");
#endif
    return 0;
}

int FatFsApp_Run(void)
{
    bsp_sd_info_t sd_info;
    DWORD free_clusters = 0U;
    FATFS *mounted_fs = NULL;
    FRESULT fr;
    uint64_t capacity_mb;

    if (!BSP_SD_IsDetected()) {
        printf("No TF card detected\r\n");
        return -1;
    }

    fr = f_mount(&sd_fs, "0:", 1);
    if (fr != FR_OK) {
        printf("f_mount failed: %s\r\n", fresult_name(fr));
        if (fr == FR_NO_FILESYSTEM) {
            printf("Card has no FAT volume. Format it as FAT32 on PC first.\r\n");
        }
        return -2;
    }

    if (BSP_SD_GetInfo(&sd_info) == BSP_SD_OK) {
        capacity_mb = ((uint64_t)sd_info.sector_count * BSP_SD_BLOCK_SIZE) / (1024ULL * 1024ULL);
        printf("Card: %lu sectors, %lu MB\r\n",
               (unsigned long)sd_info.sector_count,
               (unsigned long)capacity_mb);
    }

    printf("Mounted filesystem: %s\r\n", fs_type_name(sd_fs.fs_type));

#if APP_REQUIRE_FAT32
    if (sd_fs.fs_type != FS_FAT32) {
        printf("This project requires FAT32. Please reformat the TF card as FAT32.\r\n");
        f_mount(NULL, "0:", 0);
        return -3;
    }
#endif

    fr = f_getfree("0:", &free_clusters, &mounted_fs);
    if (fr == FR_OK && mounted_fs != NULL) {
        uint64_t free_kb = (uint64_t)free_clusters * mounted_fs->csize *
                           BSP_SD_BLOCK_SIZE / 1024ULL;
        printf("Free space: %lu KB\r\n", (unsigned long)free_kb);
    } else {
        printf("f_getfree failed: %s\r\n", fresult_name(fr));
    }

    if (print_root_dir() != 0) {
        f_mount(NULL, "0:", 0);
        return -4;
    }

    if (write_read_test() != 0) {
        f_mount(NULL, "0:", 0);
        return -5;
    }

    f_mount(NULL, "0:", 0);
    return 0;
}
