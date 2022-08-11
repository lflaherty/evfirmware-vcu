/*
 * MockStm32f7xx_hal.c
 *
 *  Created on: Oct 10 2021
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal.h"

static bool dmaInterruptsEnabled = true;

uint32_t stubITM_SendChar(uint32_t ch)
{
    (void)ch;
    return 0;
}

void stubDmaInterruptsSetEnabled(UART_HandleTypeDef* handle, bool en, uint32_t flags)
{
    (void)handle;
    (void)flags;
    dmaInterruptsEnabled = en;
}

void mockSet_HAL_DMA_IT_Enabled(bool en)
{
    dmaInterruptsEnabled = en;
}

bool mockGet_HAL_DMA_IT_Enabled(void)
{
    return dmaInterruptsEnabled;
}