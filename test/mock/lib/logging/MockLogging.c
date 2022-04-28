/*
 * MockLogging.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "MockLogging.h"

#include <stdio.h>
#include <string.h>

static char mockLogBuffer[MOCK_LOG_BUFFER_LEN]; // stores the last entry that was printed
static size_t offset = 0;

Logging_Status_T stubLogPrint(const Logging_T* logData, const char* message, const size_t len)
{
    (void)logData;

    memcpy(mockLogBuffer+offset, message, len*sizeof(char));
    offset += strnlen(message, len);

    return LOGGING_STATUS_OK;
}

Logging_Status_T stubLogPrintS(const Logging_T* logData, const char* message, const size_t bufferLen)
{
    (void)logData;

    memcpy(mockLogBuffer+offset, message, bufferLen*sizeof(char));
    offset += strnlen(message, bufferLen);

    return LOGGING_STATUS_OK;
}

void mockLogClear(void)
{
    offset = 0;
    mockLogBuffer[0] = '\0';
}

void mockLogDisplay(void)
{
    printf("%s", mockLogBuffer);
}

const char* mockLogGet(void)
{
    return mockLogBuffer;
}