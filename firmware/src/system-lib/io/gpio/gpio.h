/*
 * gpio.h
 *
 *  Created on: 19 Jul 2022
 *      Author: Liam Flaherty
 */

#ifndef IO_GPIO_GPIO_H_
#define IO_GPIO_GPIO_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct {
  GPIO_TypeDef* GPIOx;
  uint16_t GPIO_Pin;
} GPIO_T;

bool GPIO_ReadPin(GPIO_T* gpio);
void GPIO_WritePin(GPIO_T* gpio, bool high);
void GPIO_TogglePin(GPIO_T* gpio);

#endif /* IO_GPIO_GPIO_H_ */
