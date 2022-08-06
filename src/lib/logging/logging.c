/*
 * logging.c
 *
 *  Created on: 24 Jul 2021
 *      Author: Liam Flaherty
 */

#include "logging.h"
#include <stdio.h>
#include <stdbool.h>

#include "stm32f7xx_hal.h"

/**
 * Allows printing to SWO debug console
 */
int _write(int file, char *ptr, int len)
{
  (void)file; // ignore param
  
  int i=0;
  for(i = 0; i < len; i++) {
    uint32_t chr = (uint32_t)(*ptr++);
    ITM_SendChar(chr);
  }
  return len;
}

//------------------------------------------------------------------------------
Logging_Status_T Log_Init(Logging_T* log)
{
  log->mutex = xSemaphoreCreateBinaryStatic(&log->mutexBuffer);
  log->enableSerial = false;
  log->enableSWO = false;

  return LOGGING_STATUS_OK;
}

//------------------------------------------------------------------------------
Logging_Status_T Log_SetSerialStream(Logging_T* log, StreamBufferHandle_t stream)
{
  BaseType_t result = xSemaphoreTake(log->mutex, pdMS_TO_TICKS(100));
  if (result != pdTRUE) {
    return LOGGING_STATUS_ERROR_MUTEX;
  }

  log->enableSerial = true;
  log->serialOutputStream = stream;

  xSemaphoreGive(log->mutex);
  return LOGGING_STATUS_OK;
}

//------------------------------------------------------------------------------
Logging_Status_T Log_EnableSWO(Logging_T* log)
{
  BaseType_t result = xSemaphoreTake(log->mutex, pdMS_TO_TICKS(100));
  if (result != pdTRUE) {
    return LOGGING_STATUS_ERROR_MUTEX;
  }

  log->enableSWO = true;

  xSemaphoreGive(log->mutex);
  return LOGGING_STATUS_OK;
}

//------------------------------------------------------------------------------
void Log_Print(Logging_T* log, const char* msg)
{
  if (pdTRUE != xSemaphoreTake(log->mutex, pdMS_TO_TICKS(100))) {
    return;
  }

  size_t len = strnlen(msg, LOGGING_DEFAULT_BUFF_LEN);

  if (log->enableSWO) {
    if (LOGGING_DEFAULT_BUFF_LEN == len) {
      printf("[Skipping truncated message on SWO]\n");
    } else {
      printf("%s", msg);
    }
  }

  if (log->enableSerial) {
    xStreamBufferSend(log->serialOutputStream, (void*)msg, len, 0U);
  }

  xSemaphoreGive(log->mutex);
}

//------------------------------------------------------------------------------
Logging_Status_T logPrint(Logging_T* log, const char* message, const size_t len)
{
  Logging_Status_T status = LOGGING_STATUS_OK;

  (void)len;

  if (log->enableSWO) {
    printf("%s", message);
  }

  if (log->enableSerial) {
    if (pdTRUE == xSemaphoreTake(log->mutex, pdMS_TO_TICKS(100))) {
      xStreamBufferSend(log->serialOutputStream, (void*)message, len, 0U);
      xSemaphoreGive(log->mutex);
    }
  }

  return status;
}

//------------------------------------------------------------------------------
Logging_Status_T logPrintS(Logging_T* logData, const char* message, const size_t maxBufferLen)
{
  size_t len = strnlen(message, maxBufferLen);
  Logging_Status_T ret = logPrint(logData, message, len);

  if (maxBufferLen == len) {
    logPrint(logData, "Warning: truncated log message\n", LOGGING_DEFAULT_BUFF_LEN);
  }

  return ret;
}

//------------------------------------------------------------------------------
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
