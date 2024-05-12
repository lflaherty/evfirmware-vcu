/*
 * MockStm32f7xx_hal_can.c
 *
 *  Created on: 1 May 2022
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_tim.h"

// ------------------- Static data -------------------
static HAL_StatusTypeDef mStatusStart_IT = HAL_OK;

// ------------------- Methods -------------------
HAL_StatusTypeDef stubHAL_TIM_Base_Start_IT(TIM_HandleTypeDef *htim)
{
    (void)htim;

    return mStatusStart_IT;
}

void mockSet_HAL_TIME_AllStatus(HAL_StatusTypeDef status)
{
    mStatusStart_IT = status;
}

void mockSet_HAL_TIM_Base_Start_IT(HAL_StatusTypeDef status)
{
    mStatusStart_IT = status;
}
