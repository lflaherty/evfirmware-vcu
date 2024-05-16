/*
 * interrupts.c
 *
 * Global interrupt handling
 *
 *      Author: Liam Flaherty
 */

#include <stdint.h>

#include "stm32f7xx_hal.h"

#include "tasktimer/tasktimer.h"
#include "device/wheelspeed/wheelspeed.h"
#include "device/sdc/sdc.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  SDC_IRQHandler(GPIO_Pin);
}

void TIM_IRQHandler(TIM_HandleTypeDef *htim)
{
  // Advance the TaskTimer
  TaskTimer_TIM_PeriodElapsedCallback(htim);

  // Invoke wheel speed handler
  Wheelspeed_TIM_IRQHandler(htim);
}
