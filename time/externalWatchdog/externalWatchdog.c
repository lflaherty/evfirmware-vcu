/*
 * externalWatchdog.c
 *
 *  Created on: 11 Jul 2021
 *      Author: Liam Flaherty
 */

#include "externalWatchdog.h"

#include <stdio.h>

#include "stm32f7xx_hal.h"

//GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin

static GPIO_TypeDef* gpioBank;
static uint16_t gpioPin;

ExternalWatchdog_Status_T ExternalWatchdog_Init(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
  printf("ExternalWatchdog_Init begin\n");

  gpioBank = GPIOx;
  gpioPin = GPIO_Pin;

  // set pin state
  HAL_GPIO_WritePin(gpioBank, gpioPin, GPIO_PIN_RESET);

  printf("ExternalWatchdog_Init complete\n");
  return EXTWATCHDOG_STATUS_OK;
}

ExternalWatchdog_Status_T ExternalWatchdog_Trigger(void)
{
  HAL_GPIO_TogglePin(gpioBank, gpioPin);
  return EXTWATCHDOG_STATUS_OK;
}
