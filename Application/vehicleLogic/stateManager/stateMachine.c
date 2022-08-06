/*
 * stateMachine.c
 *
 *  Created on: May 11 2022
 *      Author: Liam Flaherty
 */

#include "stateMachine.h"
#include <stdio.h>

// ------------------- Private data -------------------
static Logging_T* mLog;

// ------------------- Private methods -------------------
static void stateInit(VSM_T* vsm)
{
  // Do nothing and transition on
  vsm->nextState = VSM_STATE_LV_STARTUP;

  // make sure ECU error is on
}

static void stateLvStartup(VSM_T* vsm, FaultStatus_T faultStatus)
{
  if (FAULT_FAULT == faultStatus) {
    vsm->nextState = VSM_STATE_FAULT;
  } else if (FAULT_NO_FAULT == faultStatus) {
    vsm->nextState = VSM_STATE_LV_READY;
  }
}

static void stateLvReady(VSM_T* vsm, FaultStatus_T faultStatus)
{
  if (FAULT_NO_FAULT != faultStatus) {
    vsm->nextState = VSM_STATE_FAULT;
    return;
  }

  // TODO check whether HV is on

  // Thread-safe acquisition of vehicle sense data
  bool inputBtnPressed = false;
  bool stateAccess = VehicleState_AccessAcquire(vsm->inputState);
  if (stateAccess) {
    inputBtnPressed = vsm->inputState->data.dash.buttonPressed;
  }
  VehicleState_AccessRelease(vsm->inputState);

  if (stateAccess) {
    if (inputBtnPressed && !vsm->inputButtonPrev) {
      vsm->nextState = VSM_STATE_HV_ACTIVE;
      // TODO disable ECU faults
    }

    vsm->inputButtonPrev = inputBtnPressed;
  }
}

static void stateHvActive(VSM_T* vsm, FaultStatus_T faultStatus)
{
  if (FAULT_NO_FAULT != faultStatus) {
    vsm->nextState = VSM_STATE_FAULT;
    return;
  }

  // wait and transition (if no fault)
  uint32_t currentStateMs = vsm->tickRateMs * vsm->ticksInState;
  if (currentStateMs >= vsm->vehicleConfig->vcu.hvActiveStateWait) {
    vsm->nextState = VSM_STATE_HV_CHARGING;
    VehicleControl_EnableInverter(vsm->control);
    // TODO check result ^
  }
}

static void stateHvCharging(VSM_T* vsm, FaultStatus_T faultStatus)
{
  if (FAULT_NO_FAULT != faultStatus) {
    vsm->nextState = VSM_STATE_FAULT;
    return;
  }

  // Thread-safe acquisition of vehicle sense data
  VehicleState_InverterState_T inverterState = VEHICLESTATE_INVERTERSTATE_START;
  bool stateAccess = VehicleState_AccessAcquire(vsm->inputState);
  if (stateAccess) {
    inverterState = vsm->inputState->data.inverter.state;
  }
  VehicleState_AccessRelease(vsm->inputState);

  if (stateAccess) {
    if (VEHICLESTATE_INVERTERSTATE_READY == inverterState) {
      vsm->nextState = VSM_STATE_ACTIVE_NEUTRAL;
      return;
    }

    // timeout counting
    uint32_t currentStateMs = vsm->tickRateMs * vsm->ticksInState;
    if (currentStateMs > vsm->vehicleConfig->vcu.hvChargeTimeout) {
      // HV timeout occured
      vsm->nextState = VSM_STATE_FAULT;
      return;
    }
  }
}

static void stateActiveNeutral(VSM_T* vsm, FaultStatus_T faultStatus)
{
  if (FAULT_NO_FAULT != faultStatus) {
    vsm->nextState = VSM_STATE_FAULT;
    return;
  }

  // Thread-safe acquisition of vehicle sense data
  bool inputBtnPressed = false;
  bool stateAccess = VehicleState_AccessAcquire(vsm->inputState);
  if (stateAccess) {
    inputBtnPressed = vsm->inputState->data.dash.buttonPressed;
  }
  VehicleState_AccessRelease(vsm->inputState);

  if (stateAccess) {
    if (inputBtnPressed && !vsm->inputButtonPrev) {
      // Go to forward. Set direction forward & enable torque output
      vsm->nextState = VSM_STATE_ACTIVE_FORWARD;
      ThrottleController_SetMotorDirection(vsm->throttleController, VEHICLESTATE_INVERTER_FORWARD);
      ThrottleController_SetTorqueEnabled(vsm->throttleController, true);
      // TODO check result ^
    }

    vsm->inputButtonPrev = inputBtnPressed;
  }
}

static void stateActiveForward(VSM_T* vsm, FaultStatus_T faultStatus)
{
  if (FAULT_NO_FAULT != faultStatus) {
    vsm->nextState = VSM_STATE_FAULT;
    return;
  }

  // Thread-safe acquisition of vehicle sense data
  bool inputBtnPressed = false;
  bool stateAccess = VehicleState_AccessAcquire(vsm->inputState);
  if (stateAccess) {
    inputBtnPressed = vsm->inputState->data.dash.buttonPressed;
  }
  VehicleState_AccessRelease(vsm->inputState);

  if (stateAccess) {
    if (inputBtnPressed && !vsm->inputButtonPrev) {
      // Go to reverse. Set direction reverse & enable torque output
      vsm->nextState = VSM_STATE_ACTIVE_REVERSE;
      ThrottleController_SetMotorDirection(vsm->throttleController, VEHICLESTATE_INVERTER_REVERSE);
      ThrottleController_SetTorqueEnabled(vsm->throttleController, true);
      // TODO check result ^
    }

    vsm->inputButtonPrev = inputBtnPressed;
  }
}

static void stateActiveReverse(VSM_T* vsm, FaultStatus_T faultStatus)
{
  if (FAULT_NO_FAULT != faultStatus) {
    vsm->nextState = VSM_STATE_FAULT;
    return;
  }

  // Thread-safe acquisition of vehicle sense data
  bool inputBtnPressed = false;
  bool stateAccess = VehicleState_AccessAcquire(vsm->inputState);
  if (stateAccess) {
    inputBtnPressed = vsm->inputState->data.dash.buttonPressed;
  }
  VehicleState_AccessRelease(vsm->inputState);

  if (stateAccess) {
    if (inputBtnPressed && !vsm->inputButtonPrev) {
      // Go to neutral. Set direction forward & disable torque output
      vsm->nextState = VSM_STATE_ACTIVE_NEUTRAL;
      ThrottleController_SetMotorDirection(vsm->throttleController, VEHICLESTATE_INVERTER_FORWARD);
      ThrottleController_SetTorqueEnabled(vsm->throttleController, false);
      // TODO check result ^
    }

    vsm->inputButtonPrev = inputBtnPressed;
  }
}

static void stateFault(VSM_T* vsm)
{
  ThrottleController_SetMotorDirection(vsm->throttleController, VEHICLESTATE_INVERTER_FORWARD);
  ThrottleController_SetTorqueEnabled(vsm->throttleController, false);
  // TODO assert ECU fault
}

// ------------------- Public methods -------------------
void VSM_Init(Logging_T* logger, VSM_T* vsm)
{
  mLog = logger;
  Log_Print(mLog, "VSM_Init begin\n");

  vsm->vsmState = VSM_STATE_INIT;
  vsm->nextState = VSM_STATE_INIT;
  vsm->ticksInState = 0;
  vsm->inputButtonPrev = false;

  vsm->faultMgr.vehicleConfig = vsm->vehicleConfig;
  vsm->faultMgr.tickRateMs = vsm->tickRateMs;
  FaultManager_Init(logger, &vsm->faultMgr);

  Log_Print(mLog, "VSM_Init complete\n");
}

void VSM_Step(VSM_T* vsm)
{
  // Run fault manager (common to most states)
  FaultStatus_T faultStatus = FaultManager_Step(&vsm->faultMgr);

  // run state-specific logic
  switch (vsm->vsmState) {
    case VSM_STATE_INIT:
      stateInit(vsm);
      break;
    
    case VSM_STATE_LV_STARTUP:
      stateLvStartup(vsm, faultStatus);
      break;
    
    case VSM_STATE_LV_READY:
      stateLvReady(vsm, faultStatus);
      break;
    
    case VSM_STATE_HV_ACTIVE:
      stateHvActive(vsm, faultStatus);
      break;
    
    case VSM_STATE_HV_CHARGING:
      stateHvCharging(vsm, faultStatus);
      break;
    
    case VSM_STATE_ACTIVE_NEUTRAL:
      stateActiveNeutral(vsm, faultStatus);
      break;

    case VSM_STATE_ACTIVE_FORWARD:
      stateActiveForward(vsm, faultStatus);
      break;
    
    case VSM_STATE_ACTIVE_REVERSE:
      stateActiveReverse(vsm, faultStatus);
      break;
    
    case VSM_STATE_FAULT:
      stateFault(vsm);
      break;
    
    default:
      vsm->nextState = VSM_STATE_INIT;
  }

  // Increment to next state
  ++vsm->ticksInState;

  if (vsm->vsmState != vsm->nextState) {
    vsm->ticksInState = 0;
    vsm->vsmState = vsm->nextState;
  }
}