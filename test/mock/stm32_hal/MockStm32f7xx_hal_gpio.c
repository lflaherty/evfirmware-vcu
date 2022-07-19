/*
 * MockStm32f7xx_hal_gpio.c
 *
 *  Created on: 19 Jul 2022
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_gpio.h"

#include <string.h>
#include <assert.h>

// ------------------- Static data -------------------
static bool mAsserted = false;

// ------------------- Methods -------------------
GPIO_PinState stubHAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    (void)GPIOx;
    (void)GPIO_Pin;
    return mAsserted ? GPIO_PIN_SET : GPIO_PIN_RESET;
}

void stubHAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState)
{
    (void)GPIOx;
    (void)GPIO_Pin;
    mAsserted = (PinState == GPIO_PIN_SET);
}

void stubHAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
    (void)GPIOx;
    (void)GPIO_Pin;
    mAsserted = !mAsserted;
}

void mockSet_GPIO_Asserted(bool asserted)
{
    mAsserted = asserted;
}

bool mockGet_GPIO_Asserted(void)
{
    return mAsserted;
}
