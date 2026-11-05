/** (c) 2024 UMSATS
 * @file print.c
 */

#include "main.h"

#ifdef UART_PRINT

#ifndef PRINT_UART_HANDLE
#define PRINT_UART_HANDLE huart2
#endif

extern UART_HandleTypeDef PRINT_UART_HANDLE;


// This overrides the _write system call to redirect program output to USART2
// on the STM32 L452. You need this to print to a serial terminal (e.g. PuTTY
// at 115200 baud) through the ST-Link virtual COM port. Define UART_PRINT to
// enable this instead of the SWO version.
int _write(int file, char *ptr, int len)
{
    (void)file;
    HAL_UART_Transmit(&PRINT_UART_HANDLE, (uint8_t*)ptr, len, HAL_MAX_DELAY);
    return len;
}

#else

// This overrides the _write system call to redirect program output to the SWO
// pin on the STM32 L452. You need this to print to the Serial Wire Viewer.
int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++)
    {
        ITM_SendChar(ptr[i]);
    }

    return len;
}
#endif
