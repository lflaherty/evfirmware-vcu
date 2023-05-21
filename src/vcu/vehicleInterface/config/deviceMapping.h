/*
 * deviceMapping.h
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_CONFIG_DEVICEMAPPING_H_
#define VEHICLEINTERFACE_CONFIG_DEVICEMAPPING_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include "io/adc/adc.h" /* Needed for ADC channels */
#include "io/gpio/gpio.h" /* Needed for GPIO_T */
#include "comm/uart/uart.h"
#include "comm/can/can.h"
#include "device/pdm/pdm.h"

/*
 * Include main for the auto-generated pin names
 */
#include "main.h"

/*
 * The ADC channels are read in the "rank" order defined in main.c
 * when the HAL library configures the ADC peripheral
 */
#define MAPPING_ADC_NUM_CHANNELS  ((uint16_t) 12U)

#define MAPPING_ADC_THROTTLE_1    ADC_CONVERSIONCHANNEL8
#define MAPPING_ADC_THROTTLE_2    ADC_CONVERSIONCHANNEL9
#define MAPPING_ADC_BRAKE_FRONT   ADC_CONVERSIONCHANNEL10
#define MAPPING_ADC_BRAKE_REAR    ADC_CONVERSIONCHANNEL11
#define MAPPING_ADC_MPIO1         ADC_CONVERSIONCHANNEL6
#define MAPPING_ADC_MPIO2         ADC_CONVERSIONCHANNEL7
#define MAPPING_ADC_MPIO3         ADC_CONVERSIONCHANNEL4
#define MAPPING_ADC_MPIO4         ADC_CONVERSIONCHANNEL5
#define MAPPING_ADC_MPIO5         ADC_CONVERSIONCHANNEL2
#define MAPPING_ADC_MPIO6         ADC_CONVERSIONCHANNEL3
#define MAPPING_ADC_MPIO7         ADC_CONVERSIONCHANNEL0
#define MAPPING_ADC_MPIO8         ADC_CONVERSIONCHANNEL1

/*
 * PDM Config
 */
enum PDM_Channels
{
  PDM_Ch1 = 0U,
  PDM_Ch2,
  PDM_Ch3,
  PDM_Ch4,
  PDM_Ch5,
  PDM_Ch6,
  PDM_NUM_CHANNELS
};
extern PDM_Channel_T pdmChannels[];
extern uint8_t numPdmChannels;

/*
 * GPIO Config
 */
extern GPIO_T Mapping_GPO_LED;
extern GPIO_T Mapping_GPO_DebugToggle;
extern GPIO_T Mapping_GPI_Wheelspeed_Rear;
extern GPIO_T Mapping_GPI_Wheelspeed_Front;
extern GPIO_T Mapping_GPI_StartButton;
extern GPIO_T Mapping_GPI_SDC_BMS;
extern GPIO_T Mapping_GPI_SDC_BSPD;
extern GPIO_T Mapping_GPI_SDC_IMD;
extern GPIO_T Mapping_GPI_SDC_SDCOut;
extern GPIO_T Mapping_GPO_SDC_ECUError;

/*
 * BMS config
 */
#define MAPPING_BMS_CANBUS  CAN_DEV2

/*
 * Inverter config
 */
#define MAPPING_INVERTER_CANBUS CAN_DEV1

/*
 *
 */

/*
 * PC interface config
 */
#define MAPPING_PCINTERFACE_UARTADEV  ((UART_Device_T)UART_DEV1)
#define MAPPING_PCINTERFACE_UARTBDEV  ((UART_Device_T)UART_DEV3)
extern UART_DeviceConfig_T Mapping_PCInterface_UARTA;
extern UART_DeviceConfig_T Mapping_PCInterface_UARTB;

/*
 * GPS Config
 */
#define MAPPING_GPS_UARTDEV  ((UART_Device_T)UART_DEV6)
extern GPIO_T Mapping_GPS_3dFixPin;
extern UART_DeviceConfig_T Mapping_GPS_UART;

/*
 * Getters for device handles
 */
TIM_HandleTypeDef* Mapping_GetTaskTimer100Hz(void);
TIM_HandleTypeDef* Mapping_GetTaskTimer2kHz(void);
UART_HandleTypeDef* Mapping_GetPCDebugUartA(void);
UART_HandleTypeDef* Mapping_GetPCDebugUartB(void); // Second port for ease of debugging
RTC_HandleTypeDef* Mapping_GetRTC(void);
ADC_HandleTypeDef* Mapping_GetADC(void);
CAN_HandleTypeDef* Mapping_GetCAN1(void);
CAN_HandleTypeDef* Mapping_GetCAN2(void);
CAN_HandleTypeDef* Mapping_GetCAN3(void);
CRC_HandleTypeDef* Mapping_GetCRC(void);

#endif /* VEHICLEINTERFACE_CONFIG_DEVICEMAPPING_H_ */
