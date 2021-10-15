/*
 * MockStm32f7xx_hal_adc.c
 *
 *  Created on: Oct 15 2021
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_adc.h"

#include <string.h>
#include <assert.h>

// ------------------- Static data -------------------
static HAL_StatusTypeDef mStatusStartDMA = HAL_OK;
static uint32_t mData[MOCK_ADC_MAX_DATA];
static uint32_t mDataLen = 0;

// ------------------- Methods -------------------
HAL_StatusTypeDef HAL_ADC_Start_DMA(ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length)
{
    (void)hadc;
    assert(mDataLen == Length);

    memcpy(pData, mData, mDataLen); // copy data out

    return mStatusStartDMA;
}

void mockSetADCData(uint32_t* data, size_t dataLength)
{
    assert(dataLength <= MOCK_ADC_MAX_DATA);
    memcpy(mData, data, dataLength);
    mDataLen = dataLength;
}

void mockSet_HAL_ADC_Start_DMA_Status(HAL_StatusTypeDef status)
{
    mStatusStartDMA = status;
}