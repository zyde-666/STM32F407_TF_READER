#include "usb_device.h"

#include "main.h"
#include "usbd_core.h"
#include "usbd_desc.h"
#include "usbd_msc.h"
#include "usbd_storage_if.h"

USBD_HandleTypeDef hUsbDeviceFS;

void MX_USB_DEVICE_Init(void)
{
    if (USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) {
        Error_Handler();
    }

    if (USBD_RegisterClass(&hUsbDeviceFS, USBD_MSC_CLASS) != USBD_OK) {
        Error_Handler();
    }

    if (USBD_MSC_RegisterStorage(&hUsbDeviceFS, &USBD_Storage_Interface_fops_FS) != USBD_OK) {
        Error_Handler();
    }

    if (USBD_Start(&hUsbDeviceFS) != USBD_OK) {
        Error_Handler();
    }
}
