/*
 * vehicleControl.c
 *
 *  Created on: May 6 2022
 *      Author: Liam Flaherty
 */

#include "vehicleControl.h"

#include <string.h>

#include "time/tasktimer/tasktimer.h"

// ------------------- Private data -------------------
static Logging_T* mLog;

// ------------------- Public methods -------------------
VehicleControl_Status_T VehicleControl_Init(
    Logging_T* logger,
    VehicleControl_T* control)
{
  mLog = logger;
  Log_Print(mLog, "VehicleControl_Init begin\n");
  DEPEND_ON_STATIC(SDC, VEHICLECONTROL_STATUS_ERROR_DEPENDS);

  // Nothing to do

  REGISTER(control, VEHICLECONTROL_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "VehicleControl_Init complete\n");
  return VEHICLECONTROL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_EnableInverter(VehicleControl_T* vc)
{
  (void)vc;
  // TODO
  return VEHICLECONTROL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_DisableInverter(VehicleControl_T* vc)
{
  (void)vc;
  // TODO
  return VEHICLECONTROL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_RequestMotorTorque(VehicleControl_T* vc, float torque, VehicleState_InverterDirection_T direction)
{
  (void)vc;
  (void)torque;
  (void)direction;
  // TODO
  return VEHICLECONTROL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_SetPowerChannel(VehicleControl_T* vc, uint8_t channel, bool enabled)
{
  (void)vc;
  (void)channel;
  (void)enabled;
  // TODO
  return VEHICLECONTROL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_SetECUError(VehicleControl_T* vc, bool error)
{
  (void)vc;
  SDC_AssertECUFault(error);
  return VEHICLECONTROL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_SetDash(VehicleControl_T* vc, bool ledOn)
{
  (void)vc;
  (void)ledOn;
  // TODO
  return VEHICLECONTROL_STATUS_OK;
}
