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
#include "stm32_hal/MockStm32f7xx_hal_adc.h"
#include "stm32_hal/MockStm32f7xx_hal_gpio.h"
#include "stm32_hal/MockStm32f7xx_hal_tim.h"
#include "stm32_hal/MockStm32f7xx_hal_crc.h"

#include <stdint.h>

#define portMAX_DELAY (TickType_t) 0xffffffffUL

uint32_t stubITM_SendChar(uint32_t ch);

#define ITM_SendChar stubITM_SendChar;

#endif