/*
 * cInverter.h
 *
 *  Created on: Oct 7 2021
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_INVERTER_CINVERTER_H_
#define DEVICE_INVERTER_CINVERTER_H_

#include "lib/logging/logging.h"

#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"

#include "vehicleInterface/vehicleState/vehicleState.h"

#include "cInverterCAN.h"  /* Defines offset CAN IDs */

typedef struct
{
  // ******* Setup *******
  CAN_HandleTypeDef* hcan;    // CAN device connected to inverter
  uint16_t canIdBase;         // CAN ID to offset all packet IDs from
  VehicleState_T* vehicleState;

  // ******* Internal use *******
  // Empty
} CInverter_T;

typedef enum
{
  CINVERTER_STATUS_OK         = 0x00U,
  CINVERTER_STATUS_ERROR_CAN  = 0x01U
} CInverter_Status_T;

/**
 * @brief Initialize the inverter driver
 * @param logger Pointer to logging settings
 * @param inv inverter data
 */
CInverter_Status_T CInverter_Init(Logging_T* logger, CInverter_T* inv);

// TODO add inverter control


#endif /* DEVICE_INVERTER_CINVERTER_H_ */
