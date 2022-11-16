/*
 * stateMachine.h
 *
 *  Created on: May 11 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLELOGIC_STATEMANAGER_STATEMACHINE_H_
#define VEHICLELOGIC_STATEMANAGER_STATEMACHINE_H_

#include <stdint.h>

#include "lib/logging/logging.h"
#include "vehicleInterface/vehicleState/vehicleState.h"
#include "vehicleInterface/vehicleControl/vehicleControl.h"
#include "vehicleLogic/throttleController/throttleController.h"

#include "faultManager.h"

typedef enum
{
  VSM_STATE_INIT            = 0x00U,
  VSM_STATE_FAULT           = 0x01U,
  VSM_STATE_LV_STARTUP      = 0x02U,
  VSM_STATE_LV_READY        = 0x03U,
  VSM_STATE_HV_ACTIVE       = 0x04U,
  VSM_STATE_HV_CHARGING     = 0x05U,
  VSM_STATE_ACTIVE_NEUTRAL  = 0x06U,
  VSM_STATE_ACTIVE_FORWARD  = 0x07U,
  VSM_STATE_ACTIVE_REVERSE  = 0x08U
} VSM_State_T;

typedef struct
{
  // Config
  uint32_t tickRateMs; // milliseconds per count

  // Vehicle interface
  VehicleState_T* inputState; // must be set prior to initialization
  VehicleControl_T* control; // must be set
  ThrottleController_T* throttleController; // must be set
  Config_T* vehicleConfig; // must be set

  // Internal storage
  VSM_State_T vsmState;
  VSM_State_T nextState; // for staging next state
  uint32_t ticksInState;
  bool inputButtonPrev; // used for 0->1 detections
  FaultManager_T faultMgr;
} VSM_T;

/**
 * @brief Initialize the vehicle state manager
 * @param logger Pointer to system logger
 * @param sm Pointer to VehicleStateManager object
 * @returns Success status. STATEMANAGER_STATUS_OK if successful.
 */
void VSM_Init(Logging_T* logger, VSM_T* vsm);

/**
 * @brief Initialize the vehicle state manager
 * @param logger Pointer to system logger
 * @param sm Pointer to VehicleStateManager object
 * @returns Success status. STATEMANAGER_STATUS_OK if successful.
 */
void VSM_Step(VSM_T* vsm);

#endif // VEHICLELOGIC_STATEMANAGER_STATEMACHINE_H_
