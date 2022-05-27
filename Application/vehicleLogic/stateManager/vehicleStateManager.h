/*
 * vehicleStateManager.h
 *
 *  Created on: May 11 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLELOGIC_STATEMANAGER_VEHICLESTATEMANAGER_H_
#define VEHICLELOGIC_STATEMANAGER_VEHICLESTATEMANAGER_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "lib/logging/logging.h"

typedef enum
{
  STATEMANAGER_STATUS_OK    = 0x00U
} VehicleStateManager_Status_T;

typedef struct
{
  // TODO
} VehicleStateManager_T;

/**
 * @brief Initialize the vehicle state manager
 * @param logger Pointer to system logger
 * @param sm Pointer to VehicleStateManager object
 * @returns Success status. STATEMANAGER_STATUS_OK if successful.
 */
VehicleStateManager_Status_T VehicleStateManager_Init(Logging_T* logger, VehicleStateManager_T* sm);

#endif // VEHICLELOGIC_STATEMANAGER_VEHICLESTATEMANAGER_H_ 