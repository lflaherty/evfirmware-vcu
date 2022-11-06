/*
 * MockLogging.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "MockLogging.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

static char mockLogBuffer[MOCK_LOG_BUFFER_LEN]; // stores the last entry that was printed
static size_t offset = 0;

// Mock impls ------------------------------------------------------------------
Logging_Status_T Log_Init(Logging_T* log)
{
    (void)log;
    return LOGGING_STATUS_OK;
}

Logging_Status_T Log_SetSerialStream(Logging_T* log, StreamBufferHandle_t stream)
{
    (void)log;
    (void)stream;
    return LOGGING_STATUS_OK;
}

Logging_Status_T Log_EnableSWO(Logging_T* log)
{
    (void)log;
    return LOGGING_STATUS_OK;
}

void Log_Print(Logging_T* logData, const char* message)
{
    (void)logData;

    size_t len = strnlen(message, LOGGING_MAX_MSG_LEN);
    assert(len + offset + 1U <= MOCK_LOG_BUFFER_LEN);

    memcpy(mockLogBuffer+offset, message, len + 1U);
    offset += len;
}

size_t strnlen(const char* str, const size_t maxBufferLen)
{
  size_t len = 0;
  const char* c = str;
  while (len < maxBufferLen && '\0' != *c) {
    ++c;
    ++len;
  }

  return len;
}
//------------------------------------------------------------------------------

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
