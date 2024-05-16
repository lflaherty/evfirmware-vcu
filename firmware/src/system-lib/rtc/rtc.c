/*
 * rtc.c
 *
 *  Created on: Jul 18, 2021
 *      Author: Liam Flaherty
 */

#include "rtc.h"
#include "stm32f7xx_hal.h"

#include <stdio.h>

#include "depends/depends.h"
#include "logging/logging.h"

REGISTERED_MODULE_STATIC_DEF(RTC);

// ------------------- Private data -------------------
static Logging_T* log;


// ------------------- Public methods -------------------
RTC_Status_T RTC_Init(Logging_T* logger)
{
  log = logger;
  Log_Print(log, "RTC_Init begin\n");
  DEPEND_ON(logger, RTC_STATUS_ERROR);

  // Nothing to do here

  REGISTER_STATIC(RTC, RTC_STATUS_ERROR);
  Log_Print(log, "RTC_Init complete\n");
  return RTC_STATUS_OK;
}

RTC_Status_T RTC_SetDateTime(RTC_HandleTypeDef *hrtc, RTC_DateTime_T* dateTime)
{
  if (HAL_OK != HAL_RTC_SetTime(hrtc, &dateTime->time, RTC_FORMAT_BIN)) {
    return RTC_STATUS_ERROR;
  }

  if (HAL_OK != HAL_RTC_SetDate(hrtc, &dateTime->date, RTC_FORMAT_BIN)) {
    return RTC_STATUS_ERROR;
  }

  return RTC_STATUS_OK;
}

RTC_Status_T RTC_GetDateTime(RTC_HandleTypeDef *hrtc, RTC_DateTime_T* dateTime)
{
  if (HAL_OK != HAL_RTC_GetTime(hrtc, &dateTime->time, RTC_FORMAT_BIN)) {
    return RTC_STATUS_ERROR;
  }

  if (HAL_OK != HAL_RTC_GetDate(hrtc, &dateTime->date, RTC_FORMAT_BIN)) {
    return RTC_STATUS_ERROR;
  }

  return RTC_STATUS_OK;
}
