#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#include "usbd_def.h"

#define DEVICE_FS 0U

extern USBD_HandleTypeDef hUsbDeviceFS;

void MX_USB_DEVICE_Init(void);

#endif
