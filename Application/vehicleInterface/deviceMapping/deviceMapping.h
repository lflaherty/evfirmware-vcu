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
// TODO

/*
 * Getters for device handles
 */
// TODO inlining?
TIM_HandleTypeDef* Mapping_GetTaskTimer(void);
UART_HandleTypeDef* Mapping_GetUART1(void);
RTC_HandleTypeDef* Mapping_GetRTC(void);
CAN_HandleTypeDef* Mapping_GetCAN1(void);
CAN_HandleTypeDef* Mapping_GetCAN2(void);
CAN_HandleTypeDef* Mapping_GetCAN3(void);

#endif /* VEHICLEINTERFACE_DEVICEMAPPING_DEVICEMAPPING_H_ */
