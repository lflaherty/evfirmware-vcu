/*
 * externalWatchdog.c
 *
 *  Created on: 11 Jul 2021
 *      Author: Liam Flaherty
 */

#include "externalWatchdog.h"

#include <stdio.h>

#include "stm32f7xx_hal.h"

#include "lib/logging/logging.h"

static Logging_T* log;

ExternalWatchdog_Status_T ExternalWatchdog_Init(Logging_T* logger, ExternalWatchdog_T* wdg)
{
  log = logger;
  logPrintS(log, "ExternalWatchdog_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // set pin state
  HAL_GPIO_WritePin(wdg->gpioBank, wdg->gpioPin, GPIO_PIN_RESET);

  logPrintS(log, "ExternalWatchdog_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return EXTWATCHDOG_STATUS_OK;
}

ExternalWatchdog_Status_T ExternalWatchdog_Trigger(ExternalWatchdog_T* wdg)
{
  HAL_GPIO_TogglePin(wdg->gpioBank, wdg->gpioPin);
  return EXTWATCHDOG_STATUS_OK;
}
