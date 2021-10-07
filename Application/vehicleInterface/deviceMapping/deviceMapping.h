/*
 * deviceMapping.h
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_
#define VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include "io/adc/adc.h" /* Needed for ADC_Channel_T */

/*
 * Include main for the auto-generated pin names
 */
#include "main.h"

/*
 * The ADC channels are read in the "rank" order defined in main.c
 * when the HAL library configures the ADC peripheral
 */
#define MAPPING_ADC_NUM_CHANNELS  ((uint16_t) 12U)
#define MAPPING_ADC1_CHANNEL0     ((ADC_Channel_T)  0U)
#define MAPPING_ADC1_CHANNEL1     ((ADC_Channel_T)  1U)
#define MAPPING_ADC1_CHANNEL2     ((ADC_Channel_T)  2U)
#define MAPPING_ADC1_CHANNEL3     ((ADC_Channel_T)  3U)
#define MAPPING_ADC1_CHANNEL4     ((ADC_Channel_T)  4U)
#define MAPPING_ADC1_CHANNEL5     ((ADC_Channel_T)  5U)
#define MAPPING_ADC1_CHANNEL6     ((ADC_Channel_T)  6U)
#define MAPPING_ADC1_CHANNEL7     ((ADC_Channel_T)  7U)
#define MAPPING_ADC1_CHANNEL8     ((ADC_Channel_T)  8U)
#define MAPPING_ADC1_CHANNEL9     ((ADC_Channel_T)  9U)
#define MAPPING_ADC1_CHANNEL8     ((ADC_Channel_T)  8U)
#define MAPPING_ADC1_CHANNEL14    ((ADC_Channel_T) 14U)
#define MAPPING_ADC1_CHANNEL15    ((ADC_Channel_T) 15U)

#define MAPPING_ADC_THROTTLE_1    MAPPING_ADC1_CHANNEL8
#define MAPPING_ADC_THROTTLE_2    MAPPING_ADC1_CHANNEL9
#define MAPPING_ADC_BRAKE_FRONT   MAPPING_ADC1_CHANNEL14
#define MAPPING_ADC_BRAKE_REAR    MAPPING_ADC1_CHANNEL15
#define MAPPING_ADC_MPIO1         MAPPING_ADC1_CHANNEL6
#define MAPPING_ADC_MPIO2         MAPPING_ADC1_CHANNEL7
#define MAPPING_ADC_MPIO3         MAPPING_ADC1_CHANNEL4
#define MAPPING_ADC_MPIO4         MAPPING_ADC1_CHANNEL5
#define MAPPING_ADC_MPIO5         MAPPING_ADC1_CHANNEL2
#define MAPPING_ADC_MPIO6         MAPPING_ADC1_CHANNEL3
#define MAPPING_ADC_MPIO7         MAPPING_ADC1_CHANNEL0
#define MAPPING_ADC_MPIO8         MAPPING_ADC1_CHANNEL1

/*
 * Getters for device handles
 */
TIM_HandleTypeDef* Mapping_GetTaskTimer(void);
UART_HandleTypeDef* Mapping_GetUART1(void);
RTC_HandleTypeDef* Mapping_GetRTC(void);
ADC_HandleTypeDef* Mapping_GetADC(void);
CAN_HandleTypeDef* Mapping_GetCAN1(void);
CAN_HandleTypeDef* Mapping_GetCAN2(void);
CAN_HandleTypeDef* Mapping_GetCAN3(void);

#endif /* VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_ */
