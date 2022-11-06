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

// ITM_SendChar
uint32_t stubITM_SendChar(uint32_t ch);
#define ITM_SendChar stubITM_SendChar

// Peripherals
#define CAN1    ((CAN_TypeDef*) 0UL)
#define CAN2    ((CAN_TypeDef*) 1UL)
#define CAN3    ((CAN_TypeDef*) 2UL)

// DMA
#define DMA_IT_TC   0x00000001U
#define DMA_IT_HT   0x00000002U
#define DMA_IT_TE   0x00000004U
#define DMA_IT_DME  0x00000008U
#define DMA_IT_FE   0x00000010U

void stubDmaInterruptsSetEnabled(UART_HandleTypeDef* handle, bool en, uint32_t flags);

#define __HAL_DMA_ENABLE_IT(handle,flag) stubDmaInterruptsSetEnabled(handle, true, flag)
#define __HAL_DMA_DISABLE_IT(handle,flag) stubDmaInterruptsSetEnabled(handle, false, flag)

void mockSet_HAL_DMA_IT_Enabled(bool en);
bool mockGet_HAL_DMA_IT_Enabled(void);

#endif
