/*
 * externalWatchdog.h
 *
 *  Created on: 11 Jul 2021
 *      Author: Liam Flaherty
 */

#ifndef TIME_EXTERNALWATCHDOG_EXTERNALWATCHDOG_H_
#define TIME_EXTERNALWATCHDOG_EXTERNALWATCHDOG_H_

#include "stm32f7xx_hal.h"

typedef enum
{
  EXTWATCHDOG_STATUS_OK     = 0x00U
} ExternalWatchdog_Status_T;

/**
 * @brief Initialize the external Watchdog Timer.
 */
ExternalWatchdog_Status_T ExternalWatchdog_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

/**
 * @brief Trigger the external watchdog timer to avoid reset.
 * Call this periodically.
 */
ExternalWatchdog_Status_T ExternalWatchdog_Trigger(void);

#endif /* TIME_EXTERNALWATCHDOG_EXTERNALWATCHDOG_H_ */
