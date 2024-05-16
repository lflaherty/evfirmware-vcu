/*
 * pdm.h
 *
 * Driver for integrated power distribution module.
 *
 *  Created on: 19 Nov 2022
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_PDM_PDM_H_
#define DEVICE_PDM_PDM_H_

#include <stdint.h>
#include <stdbool.h>

#include "depends/depends.h"
#include "logging/logging.h"
#include "gpio/gpio.h"
#include "vehicleInterface/vehicleState/vehicleState.h"

typedef enum
{
  PDM_STATUS_OK               = 0x00,
  PDM_STATUS_ERROR_DEPENDS    = 0x01,
  PDM_STATUS_ERROR_CONFIG     = 0x02,
  PDM_STATUS_ERROR_INVALID_CH = 0x03,
  PDM_STATUS_ERROR_STATE      = 0x04,
} PDM_Status_T;

typedef struct
{
  GPIO_T* pin;
} PDM_Channel_T;

typedef struct
{
  PDM_Channel_T* channels; // array of channels
  size_t numChannels;
  // The state storage needs to be updated to track the PDM channels
  VehicleState_T* vehicleState;

  // Internal
  bool initComplete;
  REGISTERED_MODULE();
} PDM_T;

/**
 * @brief Initialize the PDM.
 * 
 * @param logger Pointer to logging struct
 * @param pdm Pointer to PDM struct
 * @return PDM_Status_T 
 */
PDM_Status_T PDM_Init(Logging_T* logger, PDM_T* pdm);

/**
 * @brief Set the output status for a PDM channel.
 * Takes effect immediately.
 *
 * @param pdm Pointer to PDM struct
 * @param channel Index of pointer channel to control (index in pdm->channels)
 * @param state True for on.
 * @return PDM_Status_T 
 */
PDM_Status_T PDM_SetOutputEnabled(
    PDM_T* pdm,
    uint8_t channel,
    bool state);

#endif // DEVICE_PDM_PDM_H_
