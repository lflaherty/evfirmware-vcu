/*
 * deviceMapping.c
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#include "deviceMapping.h"
#include "stm32f7xx_hal.h"


extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern CAN_HandleTypeDef hcan3;

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;

extern RTC_HandleTypeDef hrtc;

extern SD_HandleTypeDef hsd1;

extern TIM_HandleTypeDef htim2;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;


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

CAN_HandleTypeDef* Mapping_GetCAN1(void)
{
  return &hcan1;
}

CAN_HandleTypeDef* Mapping_GetCAN2(void)
{
  return &hcan2;
}

CAN_HandleTypeDef* Mapping_GetCAN3(void)
{
  return &hcan3;
}
