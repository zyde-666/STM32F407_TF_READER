#include <sys/unistd.h>

#include "main.h"

int _write(int file, char *ptr, int len)
{
    (void)file;

    if (huart1.Instance == NULL) {
        return len;
    }

    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, (uint16_t)len, HAL_MAX_DELAY);
    return len;
}
