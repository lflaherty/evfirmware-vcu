/*
 * MockStdio.c
 *
 *  Created on: Aug 7 2022
 *      Author: Liam Flaherty
 */

#include "MockStdio.h"

#include <string.h>

char printfOut[PRINTF_BUFFER_LEN];
int printfOutSize = 0;

void mockClearPrintf(void)
{
    printfOutSize = 0U;
    memset(printfOut, 0U, sizeof(printfOut) / sizeof(char));
}
