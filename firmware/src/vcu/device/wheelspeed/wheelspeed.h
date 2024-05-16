/*
 * wheelspeed.h
 *
 * Driver for wheel speed sensor.
 * Implements two sensors: front and rear.
 * 
 * The wheel speed sensor is a hall effect sensor which reads circular
 * teeth on the wheel that rotate underneat the sensor. The sensor is
 * read through GPIO inputs. The rate of teeth passing (in the form of
 * low->high transitions) is converted to a rotational speed.
 * The driver will push data to the central vehicle state storage.
 * 
 *  Created on: 28 Jan 2023
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_WHEELSPEED_WHEELSPEED_H_
#define DEVICE_WHEELSPEED_WHEELSPEED_H_

#include "FreeRTOS.h"
#include "task.h"

#include "depends/depends.h"
#include "logging/logging.h"
#include "gpio/gpio.h"
#include "vehicleInterface/vehicleState/vehicleState.h"

REGISTERED_MODULE_STATIC(WHEELSPEED);

typedef enum
{
  WHEELSPEED_STATUS_OK            = 0x00,
  WHEELSPEED_STATUS_ERROR_DEPENDS = 0x01,
  WHEELSPEED_STATUS_ERROR_INIT    = 0x02,
} Wheelspeed_Status_T;

#define WHEELSPEED_STACK_SIZE 2000U
#define WHEELSPEED_TASK_PRIORITY 6U

typedef struct
{
  Logging_T* logging; // System logger
  VehicleState_T* state; // Vehicle state object to push data to

  // Input sense pins
  GPIO_T* frontWsPin;
  GPIO_T* rearWsPin;

  TIM_TypeDef* timerInstance; // 2kHz timer

  uint16_t sensorTeeth; // Number of teeth in WSS
} Wheelspeed_Config_T;

/**
 * @brief Initialize wheel speed sensor driver.
 * 
 * @param config Driver config struct
 * @return WHEELSPEED_STATUS_OK if successful. 
 */
Wheelspeed_Status_T Wheelspeed_Init(Wheelspeed_Config_T* config);

/**
 * @brief Timer interrupt. This must be called in the timer elapsed ISR.
 * 
 * @return Higher priority task woken
 */
BaseType_t Wheelspeed_TIM_IRQHandler(const TIM_HandleTypeDef* htim);

#endif // DEVICE_WHEELSPEED_WHEELSPEED_H_
