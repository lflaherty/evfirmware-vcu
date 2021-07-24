/*
 * logging.c
 *
 *  Created on: 24 Jul 2021
 *      Author: Liam Flaherty
 */

#include "logging.h"
#include <stdio.h>

/**
 * Allows printing to SWO debug console
 */
int _write(int file, char *ptr, int len)
{
  /* Implement your write code here, this is used by puts and printf for example */
  int i=0;
  for(i=0 ; i<len ; i++)
    ITM_SendChar((*ptr++));
  return len;
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
Logging_Status_T logPrint(const Logging_T* logData, const char* message, const size_t len)
{
  Logging_Status_T status = LOGGING_STATUS_OK;

  (void)len;

  if (logData->enableLogToDebug) {
    printf(message);
  }

  if (logData->enableLogToSerial && NULL != logData->handleSerial) {
    // TODO
  }

  if (logData->enableLogToLogFile) {
    // TODO
  }

  return status;
}

//------------------------------------------------------------------------------
Logging_Status_T logPrintS(const Logging_T* logData, const char* message, const size_t maxBufferLen)
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
