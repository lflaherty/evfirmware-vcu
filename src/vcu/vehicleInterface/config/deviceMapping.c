/*
 * deviceMapping.c
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#include "deviceMapping.h"
#include "stm32f7xx_hal.h"

/*
 * PDM Config
 */
static GPIO_T pinPdmChannel1 = { .GPIOx = P_SW1_GPIO_Port, .GPIO_Pin = P_SW1_Pin };
static GPIO_T pinPdmChannel2 = { .GPIOx = P_SW2_GPIO_Port, .GPIO_Pin = P_SW2_Pin };
static GPIO_T pinPdmChannel3 = { .GPIOx = P_SW3_GPIO_Port, .GPIO_Pin = P_SW3_Pin };
static GPIO_T pinPdmChannel4 = { .GPIOx = P_SW4_GPIO_Port, .GPIO_Pin = P_SW4_Pin };
static GPIO_T pinPdmChannel5 = { .GPIOx = P_SW5_GPIO_Port, .GPIO_Pin = P_SW5_Pin };
static GPIO_T pinPdmChannel6 = { .GPIOx = P_SW6_GPIO_Port, .GPIO_Pin = P_SW6_Pin };
PDM_Channel_T pdmChannels[] = {
  [PDM_Ch1] = {.pin = &pinPdmChannel1 },
  [PDM_Ch2] = {.pin = &pinPdmChannel2 },
  [PDM_Ch3] = {.pin = &pinPdmChannel3 },
  [PDM_Ch4] = {.pin = &pinPdmChannel4 },
  [PDM_Ch5] = {.pin = &pinPdmChannel5 },
  [PDM_Ch6] = {.pin = &pinPdmChannel6 },
};
uint8_t numPdmChannels = sizeof(pdmChannels) / sizeof(PDM_Channel_T);

/*
 * GPIO Config
 */
GPIO_T Mapping_GPIO_LED = {
  .GPIOx = STATUS_LED_GPIO_Port, .GPIO_Pin = STATUS_LED_Pin
};

/*
 * GPS Config
 */
GPIO_T Mapping_GPS_3dFixPin = {
  .GPIOx = GPS_FIX_GPIO_Port, .GPIO_Pin = GPS_FIX_Pin
};
extern UART_HandleTypeDef huart6;
UART_HandleTypeDef* Mapping_GPS_GetUARTHandle(void)
{
  return &huart6;
}

/*
 * Device handles
 */
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern CAN_HandleTypeDef hcan3;

extern CRC_HandleTypeDef hcrc;

extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c4;

extern RTC_HandleTypeDef hrtc;

extern TIM_HandleTypeDef htim2;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef huart6;


TIM_HandleTypeDef* Mapping_GetTaskTimer100Hz(void)
{
  return &htim2;
}

UART_HandleTypeDef* Mapping_GetPCDebugUartA(void)
{
  return &huart1;
}

UART_HandleTypeDef* Mapping_GetPCDebugUartB(void)
{
  return &huart3;
}

RTC_HandleTypeDef* Mapping_GetRTC(void)
{
  return &hrtc;
}

ADC_HandleTypeDef* Mapping_GetADC(void)
{
  return &hadc1;
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

CRC_HandleTypeDef* Mapping_GetCRC(void)
{
  return &hcrc;
}