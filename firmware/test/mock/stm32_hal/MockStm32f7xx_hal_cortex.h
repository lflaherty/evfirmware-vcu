/*
 * MockStm32f7xx_hal_cortex.h
 *
 *  Created on: 15 Apr 2023
 *      Author: Liam Flaherty
 */

#ifndef MOCK_STM32F7xx_HAL_CORTEX_H_
#define MOCK_STM32F7xx_HAL_CORTEX_H_

#include "stm32_hal/stm32f767xx.h"
#include <stdbool.h>

void HAL_NVIC_EnableIRQ(IRQn_Type IRQn);
void HAL_NVIC_DisableIRQ(IRQn_Type IRQn);

/**
 * @brief Gets whether an IRQ is enabled.
 * 
 * @return bool IRQ is enabled
 */
bool mockGet_HAL_Cortex_IRQEnabled(IRQn_Type IRQn);

#endif // MOCK_STM32F7xx_HAL_CORTEX_H_
