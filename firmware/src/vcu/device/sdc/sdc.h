/*
 * sdc.h
 *
 * Driver for vehicle shutdown circuit.
 * 
 *  Created on: 13 Nov 2022
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_SDC_SDC_H_
#define DEVICE_SDC_SDC_H_

#include <stdint.h>
#include <stdbool.h>

#include "depends/depends.h"
#include "logging/logging.h"
#include "gpio/gpio.h"
#include "vehicleInterface/vehicleState/vehicleState.h"

REGISTERED_MODULE_STATIC(SDC);

typedef enum
{
  SDC_STATUS_OK = 0x00,
  SDC_STATUS_ERROR_DEPENDS = 0x01,
} SDC_Status_T;

typedef struct
{
  VehicleState_T* state; // Vehicle state object to push data to

  // Input pins
  GPIO_T* pinBMS;
  GPIO_T* pinBSPD;
  GPIO_T* pinIMD;
  GPIO_T* pinSDCOut;

  // Output pin
  GPIO_T* pinECUError;
} SDC_Config_T;

#define SDC_STACK_SIZE 2000U
#define SDC_TASK_PRIORITY 4U

/**
 * @brief Initialize the shutdown-circuit driver.
 * 
 * @param logger Pointer to system logger
 * @param config Config struct
 * @return SDC_STATUS_OK if init successful 
 */
SDC_Status_T SDC_Init(Logging_T* logger, SDC_Config_T* config);

/**
 * @brief Update the ECU fault outupt status.
 * 
 * @param fault Fault status
 * @return SDC_STATUS_OK if successful 
 */
SDC_Status_T SDC_AssertECUFault(const bool fault);

void SDC_IRQHandler(uint16_t pin);

#endif // DEVICE_SDC_SDC_H_
