/*
 * externalWatchdog.h
 *
 *  Created on: 11 Jul 2021
 *      Author: Liam Flaherty
 */

#ifndef TIME_EXTERNALWATCHDOG_EXTERNALWATCHDOG_H_
#define TIME_EXTERNALWATCHDOG_EXTERNALWATCHDOG_H_

#include "lib/logging/logging.h"

#include "stm32f7xx_hal.h"

typedef struct
{
  GPIO_TypeDef* gpioBank; // GPIO Port used for WDG
  uint16_t gpioPin;       // GPIO Pin used for WDG
} ExternalWatchdog_T;

typedef enum
{
  EXTWATCHDOG_STATUS_OK     = 0x00U
} ExternalWatchdog_Status_T;

/**
 * @brief Initialize the external Watchdog Timer.
 * @param logger Pointer to logging settings
 * @param GPIOx GPIO bank for ext WDG trigger
 * @param GPIO_Pin GPIO pin for ext WDG trigger
 */
ExternalWatchdog_Status_T ExternalWatchdog_Init(Logging_T* logger, ExternalWatchdog_T* wdg);

/**
 * @brief Trigger the external watchdog timer to avoid reset.
 * Call this periodically.
 */
ExternalWatchdog_Status_T ExternalWatchdog_Trigger(ExternalWatchdog_T* wdg);

#endif /* TIME_EXTERNALWATCHDOG_EXTERNALWATCHDOG_H_ */
