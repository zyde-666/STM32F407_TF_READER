#include "usbd_conf.h"

#include "usbd_core.h"
#include "usbd_msc.h"

PCD_HandleTypeDef hpcd_USB_OTG_FS;

USBD_StatusTypeDef USBD_LL_Init(USBD_HandleTypeDef *pdev)
{
    hpcd_USB_OTG_FS.Instance = USB_OTG_FS;
    hpcd_USB_OTG_FS.Init.dev_endpoints = 4U;
    hpcd_USB_OTG_FS.Init.speed = PCD_SPEED_FULL;
    hpcd_USB_OTG_FS.Init.dma_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
    hpcd_USB_OTG_FS.Init.Sof_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.low_power_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.lpm_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.vbus_sensing_enable = DISABLE;
    hpcd_USB_OTG_FS.Init.use_dedicated_ep1 = DISABLE;
    hpcd_USB_OTG_FS.Init.use_external_vbus = DISABLE;
    hpcd_USB_OTG_FS.Init.ep0_mps = USB_OTG_MAX_EP0_SIZE;

    hpcd_USB_OTG_FS.pData = pdev;
    pdev->pData = &hpcd_USB_OTG_FS;

    if (HAL_PCD_Init(&hpcd_USB_OTG_FS) != HAL_OK) {
        return USBD_FAIL;
    }

    HAL_PCDEx_SetRxFiFo(&hpcd_USB_OTG_FS, 0x80U);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 0U, 0x40U);
    HAL_PCDEx_SetTxFiFo(&hpcd_USB_OTG_FS, 1U, 0x80U);

    return USBD_OK;
}

USBD_StatusTypeDef USBD_LL_DeInit(USBD_HandleTypeDef *pdev)
{
    (void)pdev;
    return (HAL_PCD_DeInit(&hpcd_USB_OTG_FS) == HAL_OK) ? USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_Start(USBD_HandleTypeDef *pdev)
{
    (void)pdev;
    return (HAL_PCD_Start(&hpcd_USB_OTG_FS) == HAL_OK) ? USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_Stop(USBD_HandleTypeDef *pdev)
{
    (void)pdev;
    return (HAL_PCD_Stop(&hpcd_USB_OTG_FS) == HAL_OK) ? USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_OpenEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr,
                                  uint8_t ep_type, uint16_t ep_mps)
{
    (void)pdev;
    return (HAL_PCD_EP_Open(&hpcd_USB_OTG_FS, ep_addr, ep_mps, ep_type) == HAL_OK) ?
           USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_CloseEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    (void)pdev;
    return (HAL_PCD_EP_Close(&hpcd_USB_OTG_FS, ep_addr) == HAL_OK) ? USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_FlushEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    (void)pdev;
    return (HAL_PCD_EP_Flush(&hpcd_USB_OTG_FS, ep_addr) == HAL_OK) ? USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_StallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    (void)pdev;
    return (HAL_PCD_EP_SetStall(&hpcd_USB_OTG_FS, ep_addr) == HAL_OK) ? USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_ClearStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    (void)pdev;
    return (HAL_PCD_EP_ClrStall(&hpcd_USB_OTG_FS, ep_addr) == HAL_OK) ? USBD_OK : USBD_FAIL;
}

uint8_t USBD_LL_IsStallEP(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    PCD_HandleTypeDef *hpcd = (PCD_HandleTypeDef *)pdev->pData;
    uint8_t epnum = ep_addr & 0x7FU;

    if ((ep_addr & 0x80U) == 0x80U) {
        return hpcd->IN_ep[epnum].is_stall;
    }
    return hpcd->OUT_ep[epnum].is_stall;
}

USBD_StatusTypeDef USBD_LL_SetUSBAddress(USBD_HandleTypeDef *pdev, uint8_t dev_addr)
{
    (void)pdev;
    return (HAL_PCD_SetAddress(&hpcd_USB_OTG_FS, dev_addr) == HAL_OK) ? USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_Transmit(USBD_HandleTypeDef *pdev, uint8_t ep_addr,
                                    uint8_t *pbuf, uint32_t size)
{
    (void)pdev;
    return (HAL_PCD_EP_Transmit(&hpcd_USB_OTG_FS, ep_addr, pbuf, size) == HAL_OK) ?
           USBD_OK : USBD_FAIL;
}

USBD_StatusTypeDef USBD_LL_PrepareReceive(USBD_HandleTypeDef *pdev, uint8_t ep_addr,
                                          uint8_t *pbuf, uint32_t size)
{
    (void)pdev;
    return (HAL_PCD_EP_Receive(&hpcd_USB_OTG_FS, ep_addr, pbuf, size) == HAL_OK) ?
           USBD_OK : USBD_FAIL;
}

uint32_t USBD_LL_GetRxDataSize(USBD_HandleTypeDef *pdev, uint8_t ep_addr)
{
    (void)pdev;
    return HAL_PCD_EP_GetRxCount(&hpcd_USB_OTG_FS, ep_addr);
}

void *USBD_static_malloc(uint32_t size)
{
    static uint32_t mem[(sizeof(USBD_MSC_BOT_HandleTypeDef) / 4U) + 1U];
    (void)size;
    return mem;
}

void USBD_static_free(void *p)
{
    (void)p;
}

void USBD_LL_Delay(uint32_t delay)
{
    HAL_Delay(delay);
}

void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_SetupStage((USBD_HandleTypeDef *)hpcd->pData, (uint8_t *)hpcd->Setup);
}

void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_DataOutStage((USBD_HandleTypeDef *)hpcd->pData, epnum,
                         hpcd->OUT_ep[epnum].xfer_buff);
}

void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_DataInStage((USBD_HandleTypeDef *)hpcd->pData, epnum,
                        hpcd->IN_ep[epnum].xfer_buff);
}

void HAL_PCD_SOFCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_SOF((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_ResetCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_SetSpeed((USBD_HandleTypeDef *)hpcd->pData, USBD_SPEED_FULL);
    USBD_LL_Reset((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_SuspendCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_Suspend((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_ResumeCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_Resume((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_IsoOUTIncomplete((USBD_HandleTypeDef *)hpcd->pData, epnum);
}

void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef *hpcd, uint8_t epnum)
{
    USBD_LL_IsoINIncomplete((USBD_HandleTypeDef *)hpcd->pData, epnum);
}

void HAL_PCD_ConnectCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_DevConnected((USBD_HandleTypeDef *)hpcd->pData);
}

void HAL_PCD_DisconnectCallback(PCD_HandleTypeDef *hpcd)
{
    USBD_LL_DevDisconnected((USBD_HandleTypeDef *)hpcd->pData);
}
