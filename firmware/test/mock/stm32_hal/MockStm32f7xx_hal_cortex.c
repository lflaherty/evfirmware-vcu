/*
 * MockStm32f7xx_hal_cortex.c
 *
 *  Created on: 15 Apr 2023
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_cortex.h"

#include <stdbool.h>
#include <assert.h>

static bool interruptEnabled[STM32F767_IRQMax] = { 0 };


void HAL_NVIC_EnableIRQ(IRQn_Type IRQn)
{
  // Mock doesn't support Cortex-M7 exceptions
  assert(IRQn >= 0);
  assert(IRQn < STM32F767_IRQMax);

  interruptEnabled[IRQn] = 1;
}

void HAL_NVIC_DisableIRQ(IRQn_Type IRQn)
{
  // Mock doesn't support Cortex-M7 exceptions
  assert(IRQn >= 0);
  assert(IRQn < STM32F767_IRQMax);

  interruptEnabled[IRQn] = 0;
}

bool mockGet_HAL_Cortex_IRQEnabled(IRQn_Type IRQn)
{
  // Mock doesn't support Cortex-M7 exceptions
  assert(IRQn >= 0);
  assert(IRQn < STM32F767_IRQMax);

  return interruptEnabled[IRQn];
}
