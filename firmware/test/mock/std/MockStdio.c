/*
 * MockStdio.c
 *
 *  Created on: Aug 7 2022
 *      Author: Liam Flaherty
 */

#include "MockStdio.h"

#include <string.h>
#include <stdarg.h>

char printfOut[PRINTF_BUFFER_LEN];
int printfOutSize = 0;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-nonliteral"
int mockPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    
    int i = vsnprintf(printfOut + printfOutSize, PRINTF_BUFFER_LEN, format, args);
    printfOutSize += i;

    va_end(args);

    return i;
}
#pragma GCC diagnostic pop

void mockClearPrintf(void)
{
    printfOutSize = 0U;
    memset(printfOut, 0U, sizeof(printfOut) / sizeof(char));
}
