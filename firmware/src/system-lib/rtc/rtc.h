/*
 * rtc.h
 *
 *  Created on: Jul 18, 2021
 *      Author: Liam Flaherty
 */

#ifndef TIME_RTC_RTC_H_
#define TIME_RTC_RTC_H_

#include "stm32f7xx_hal.h"

#include "depends/depends.h"
#include "logging/logging.h"

REGISTERED_MODULE_STATIC(RTC);

typedef enum
{
  RTC_STATUS_OK     = 0x00U,
  RTC_STATUS_ERROR  = 0x01U
} RTC_Status_T;

typedef struct
{
  RTC_TimeTypeDef time;
  RTC_DateTypeDef date;
} RTC_DateTime_T;

/**
 * Initialize RTC
 * @param logger Pointer to logging settings
 */
RTC_Status_T RTC_Init(Logging_T* logger);

/**
 * Update the internal date & time
 */
RTC_Status_T RTC_SetDateTime(RTC_HandleTypeDef *hrtc, RTC_DateTime_T* dateTime);

/**
 * Sets the date and time
 */
RTC_Status_T RTC_GetDateTime(RTC_HandleTypeDef *hrtc, RTC_DateTime_T* dateTime);



#endif /* TIME_RTC_RTC_H_ */
