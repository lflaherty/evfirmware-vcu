/*
 * MockStm32f7xx_hal.h
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#ifndef MOCK_STM32HAL_MOCKSTM32FXXHAL_H_
#define MOCK_STM32HAL_MOCKSTM32FXXHAL_H_

#include "stm32_hal/MockStm32f7xx_hal_uart.h"
#include "stm32_hal/MockStm32f7xx_hal_can.h"

static inline uint32_t ITM_SendChar (uint32_t ch)
{
  (void)ch;
  return 0;
}

#endif