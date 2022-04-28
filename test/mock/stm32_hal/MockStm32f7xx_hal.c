/*
 * MockStm32f7xx_hal.c
 *
 *  Created on: Oct 10 2021
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal.h"

uint32_t stubITM_SendChar(uint32_t ch)
{
    (void)ch;
    return 0;
}