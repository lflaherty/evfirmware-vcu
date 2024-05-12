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
static uint32_t* mDataPtr = NULL;
static uint32_t mDataLen = 0; // Number of elements in mDataPtr (not number of bytes)

// ------------------- Methods -------------------
HAL_StatusTypeDef stubHAL_ADC_Start_DMA(ADC_HandleTypeDef* hadc, uint32_t* pData, uint32_t Length)
{
    (void)hadc;

    mDataPtr = pData;
    mDataLen = Length;

    return mStatusStartDMA;
}

void mockSetADCData(uint32_t* data, uint32_t dataLength)
{
    assert(mDataPtr != NULL);
    assert(mDataLen >= dataLength);

    memcpy(mDataPtr, data, dataLength * sizeof(uint32_t));
    mDataLen = dataLength;
}

void mockSetADCDataChannel(const uint32_t channel, const uint32_t val)
{
    assert(mDataPtr != NULL);
    assert(channel <= mDataLen);

    mDataPtr[channel] = val;
}

void mockClearADCClear(void)
{
    memset(mDataPtr, 0, mDataLen * sizeof(uint32_t));
}

void mockSet_HAL_ADC_Start_DMA_Status(HAL_StatusTypeDef status)
{
    mStatusStartDMA = status;
}
