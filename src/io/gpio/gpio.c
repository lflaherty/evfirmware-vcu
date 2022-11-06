/*
 * gpio.c
 *
 *  Created on: 19 Jul 2022
 *      Author: Liam Flaherty
 */

#include "gpio.h"

#include "stm32f7xx_hal.h"
#include <stdint.h>

bool GPIO_ReadPin(GPIO_T* gpio)
{
  return HAL_GPIO_ReadPin(gpio->GPIOx, gpio->GPIO_Pin);
}

void GPIO_WritePin(GPIO_T* gpio, bool asserted)
{
  HAL_GPIO_WritePin(gpio->GPIOx, gpio->GPIO_Pin, asserted);
}

void GPIO_TogglePin(GPIO_T* gpio)
{
  HAL_GPIO_TogglePin(gpio->GPIOx, gpio->GPIO_Pin);
}
