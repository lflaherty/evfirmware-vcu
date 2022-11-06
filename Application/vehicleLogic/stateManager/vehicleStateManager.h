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

#include "vehicleInterface/vehicleState/vehicleState.h"
#include "vehicleInterface/vehicleControl/vehicleControl.h"
#include "vehicleLogic/throttleController/throttleController.h"
#include "stateMachine.h"

typedef enum
{
  STATEMANAGER_STATUS_OK          = 0x00U,
  STATEMANAGER_STATUS_ERROR_INIT  = 0x01U
} VehicleStateManager_Status_T;

#define VEHICLESTATEMANAGER_STACK_SIZE 2000
#define VEHICLESTATEMANAGER_TASK_PRIORITY 3

typedef struct
{
  // Config
  VehicleState_T* inputState; // must be set prior to initialization
  VehicleControl_T* control; // must be set prior to initialization
  ThrottleController_T* throttleController; // must be set prior to initialization
  Config_T* vehicleConfig; // must be set prior to initialization

  // ******* Internal use *******
  VSM_T vsm;

  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[VEHICLESTATEMANAGER_STACK_SIZE];
} VehicleStateManager_T;

/**
 * @brief Initialize the vehicle state manager
 * @param logger Pointer to system logger
 * @param sm Pointer to VehicleStateManager object
 * @returns Success status. STATEMANAGER_STATUS_OK if successful.
 */
VehicleStateManager_Status_T VehicleStateManager_Init(Logging_T* logger, VehicleStateManager_T* sm);

#endif // VEHICLELOGIC_STATEMANAGER_VEHICLESTATEMANAGER_H_
