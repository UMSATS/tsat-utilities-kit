/** (c) 2024 UMSATS
 * @file print.c
 *
 * @date Aug 11, 2024
 */

#include "main.h"

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
