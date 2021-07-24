/*
 * rtc.c
 *
 *  Created on: Jul 18, 2021
 *      Author: Liam Flaherty
 */

#include "rtc.h"
#include "stm32f7xx_hal.h"

#include <stdio.h>


RTC_Status_T RTC_Init(void)
{
  printf("RTC_Init begin\n");

  // Nothing to do here

  printf("RTC_Init complete\n");
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
