/*
 * MockStm32f7xx_hal_adc.h
 * Some excerpts from stm32f7xx_hal_adc.h in STM32 HAL.
 *
 *  Created on: Oct 15 2021
 *      Author: Liam Flaherty
 */

#ifndef MOCK_STM32F7xx_ADC_H_
#define MOCK_STM32F7xx_ADC_H_

#include <stdint.h>
#include <stddef.h>
#include "MockStm32f7xx_hal_def.h"

#define MOCK_ADC_MAX_DATA 64

// ================== Define types ==================
typedef struct
{
    // Empty for mock
} ADC_TypeDef;

typedef struct
{
    ADC_TypeDef* Instance;
} ADC_HandleTypeDef;


// ================== Define methods ==================
HAL_StatusTypeDef stubHAL_ADC_Start_DMA(ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length);

// Replace real methods with mock stubs
#define HAL_ADC_Start_DMA stubHAL_ADC_Start_DMA

// ================== Mock control methods ==================
void mockSetADCData(uint32_t* data, size_t dataLength);
void mockSet_HAL_ADC_Start_DMA_Status(HAL_StatusTypeDef status);

#endif