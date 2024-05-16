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
_Static_assert(PDM_NUM_CHANNELS == sizeof(pdmChannels)/sizeof(pdmChannels[0]),
    "array must be same size as header specifies");

/*
 * GPIO Config
 */
GPIO_T Mapping_GPO_LED = {
  .GPIOx = STATUS_LED_GPIO_Port, .GPIO_Pin = STATUS_LED_Pin
};
GPIO_T Mapping_GPO_DebugToggle = {
  .GPIOx = EXP_GPIO1_GPIO_Port, .GPIO_Pin = EXP_GPIO1_Pin
};
GPIO_T Mapping_GPI_Wheelspeed_Rear = {
  .GPIOx = DIN_WS_R_GPIO_Port, .GPIO_Pin = DIN_WS_R_Pin
};
GPIO_T Mapping_GPI_Wheelspeed_Front = {
  .GPIOx = DIN_WS_F_GPIO_Port, .GPIO_Pin = DIN_WS_F_Pin
};
GPIO_T Mapping_GPI_StartButton = {
  .GPIOx = DIN_START_B_GPIO_Port, .GPIO_Pin = DIN_START_B_Pin
};
GPIO_T Mapping_GPI_SDC_BMS = {
  .GPIOx = GPIO_EXTI9_SDCBMS_GPIO_Port, .GPIO_Pin = GPIO_EXTI9_SDCBMS_Pin
};
GPIO_T Mapping_GPI_SDC_BSPD = {
  .GPIOx = GPIO_EXTI15_SDCBSPD_GPIO_Port, .GPIO_Pin = GPIO_EXTI15_SDCBSPD_Pin
};
GPIO_T Mapping_GPI_SDC_IMD = {
  .GPIOx = GPIO_EXTI10_SDCIMD_GPIO_Port, .GPIO_Pin = GPIO_EXTI10_SDCIMD_Pin
};
GPIO_T Mapping_GPI_SDC_SDCOut = {
  .GPIOx = GPIO_EXTI4_SDCOUT_GPIO_Port, .GPIO_Pin = GPIO_EXTI4_SDCOUT_Pin
};
GPIO_T Mapping_GPO_SDC_ECUError = {
  .GPIOx = ECU_SW_Error_GPIO_Port, .GPIO_Pin = ECU_SW_Error_Pin
};

/*
 * PC interface config
 */
UART_DeviceConfig_T Mapping_PCInterface_UARTA = {
  .dev = MAPPING_PCINTERFACE_UARTADEV,
  .handle = &huart1,
  .txIrq = DMA2_Stream7_IRQn,
};
UART_DeviceConfig_T Mapping_PCInterface_UARTB = {
  .dev = MAPPING_PCINTERFACE_UARTBDEV,
  .handle = &huart3,
  .txIrq = DMA1_Stream3_IRQn,
};

/*
 * GPS Config
 */
GPIO_T Mapping_GPS_3dFixPin = {
  .GPIOx = GPS_FIX_GPIO_Port, .GPIO_Pin = GPS_FIX_Pin
};
UART_DeviceConfig_T Mapping_GPS_UART = {
  .dev = MAPPING_GPS_UARTDEV,
  .handle = &huart6,
  .txIrq = DMA2_Stream6_IRQn,
};

