#ifndef MAIN_H
#define MAIN_H

#include "stm32f4xx_hal.h"

extern UART_HandleTypeDef huart1;

void SystemClock_Config(void);
void Error_Handler(void);

#endif
