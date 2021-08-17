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

static GPIO_TypeDef* gpioBank;
static uint16_t gpioPin;

ExternalWatchdog_Status_T ExternalWatchdog_Init(Logging_T* logger, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  log = logger;
  logPrintS(log, "ExternalWatchdog_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  gpioBank = GPIOx;
  gpioPin = GPIO_Pin;

  // set pin state
  HAL_GPIO_WritePin(gpioBank, gpioPin, GPIO_PIN_RESET);

  logPrintS(log, "ExternalWatchdog_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return EXTWATCHDOG_STATUS_OK;
}

ExternalWatchdog_Status_T ExternalWatchdog_Trigger(void)
{
  HAL_GPIO_TogglePin(gpioBank, gpioPin);
  return EXTWATCHDOG_STATUS_OK;
}
