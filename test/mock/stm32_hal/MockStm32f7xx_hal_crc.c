/*
 * MockStm32f7xx_hal_crc.c
 *
 *  Created on: Jul 28 2022
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_crc.h"

#include <string.h>
#include <assert.h>

// ------------------- Static data -------------------
static uint32_t mCrcValue = 0U;

// ------------------- Methods -------------------
uint32_t stubHAL_CRC_Calculate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength)
{
    (void)hcrc;
    (void)pBuffer;
    (void)BufferLength;
    return mCrcValue;
}

void mockSet_CRC(uint32_t crc)
{
    mCrcValue = crc;
}