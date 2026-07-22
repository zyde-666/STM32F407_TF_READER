#include "main.h"

#include "app_config.h"

#if APP_ENABLE_USB_MSC
extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
#endif

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    Error_Handler();
}

void MemManage_Handler(void)
{
    Error_Handler();
}

void BusFault_Handler(void)
{
    Error_Handler();
}

void UsageFault_Handler(void)
{
    Error_Handler();
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

#if APP_ENABLE_USB_MSC
void OTG_FS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
#endif
