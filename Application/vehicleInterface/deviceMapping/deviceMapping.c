/*
 * deviceMapping.c
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#include "deviceMapping.h"
#include "stm32f7xx_hal.h"


extern TIM_HandleTypeDef htim2;

extern RTC_HandleTypeDef hrtc;

extern UART_HandleTypeDef huart1;

TIM_HandleTypeDef* Mapping_GetTaskTimer(void)
{
  return &htim2;
}


UART_HandleTypeDef* Mapping_GetUART1(void)
{
  return &huart1;
}


RTC_HandleTypeDef* Mapping_GetRTC(void)
{
  return &hrtc;
}

