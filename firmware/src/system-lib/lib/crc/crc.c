/*
 * crc.c
 *
 *  Created on: 20 Oct 2022
 *      Author: Liam Flaherty
 */

#include "crc.h"

// ------------------- Private data -------------------
static Logging_T* mLog;

CRC_Status_T CRC_Init(Logging_T* logger, CRC_T* crcObj)
{
  mLog = logger;
  Log_Print(mLog, "CRC_Init begin\n");
  DEPEND_ON(logger, CRC_STATUS_ERROR_DEPENDS);

  // Create mutex lock
  crcObj->mutex = xSemaphoreCreateMutexStatic(&crcObj->mutexBuffer);

  REGISTER(crcObj, CRC_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "complete begin\n");
  return CRC_STATUS_OK;
}

bool CRC_Calculate(
    CRC_T* crcObj,
    uint32_t buffer[],
    uint32_t bufferLen,
    TickType_t timeout,
    uint32_t* crcOut)
{
  BaseType_t locked = xSemaphoreTake(crcObj->mutex, timeout);
  if (pdTRUE != locked) {
    *crcOut = 0U;
    return false;
  }

  *crcOut = HAL_CRC_Calculate(crcObj->hcrc, buffer, bufferLen);

  xSemaphoreGive(crcObj->mutex);
  return true;
}
