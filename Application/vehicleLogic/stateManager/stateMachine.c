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
  // TODO move 3000 to configurable constant
  if (currentStateMs >= 3000U) {
    vsm->nextState = VSM_STATE_HV_CHARGING;
    VehicleControl_EnableInverter(vsm->control);
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
    if (currentStateMs > vsm->hvChargeTimeoutMs) {
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
      vsm->nextState = VSM_STATE_ACTIVE_FORWARD;
      // TODO enable torque output, set direction forward
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
      vsm->nextState = VSM_STATE_ACTIVE_REVERSE;
      // TODO enable torque output, set direction reverse
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
      vsm->nextState = VSM_STATE_ACTIVE_NEUTRAL;
      // TODO disable torque output, set direction forward
    }

    vsm->inputButtonPrev = inputBtnPressed;
  }
}

static void stateFault(VSM_T* vsm)
{
  (void)vsm;
  // Do nothing
}

// ------------------- Public methods -------------------
void VSM_Init(Logging_T* logger, VSM_T* vsm)
{
  mLog = logger;
  logPrintS(mLog, "VSM_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  vsm->vsmState = VSM_STATE_INIT;
  vsm->nextState = VSM_STATE_INIT;
  vsm->ticksInState = 0;
  vsm->inputButtonPrev = false;

  // TODO write up vsm->faultMgr
  FaultManager_Init(logger, &vsm->faultMgr);

  logPrintS(mLog, "VSM_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
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