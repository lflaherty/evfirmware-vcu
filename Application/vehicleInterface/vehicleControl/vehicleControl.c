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
VehicleControl_Status_T VehicleControl_Init(Logging_T* logger)
{
  mLog = logger;
  Log_Print(mLog, "VehicleControl_Init begin\n");

  // Nothing to do

  Log_Print(mLog, "VehicleControl_Init complete\n");
  return VEHICLECONTORL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_EnableInverter(VehicleControl_T* vc)
{
  (void)vc;
  // TODO
  return VEHICLECONTORL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_DisableInverter(VehicleControl_T* vc)
{
  (void)vc;
  // TODO
  return VEHICLECONTORL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_RequestMotorTorque(VehicleControl_T* vc, float torque, VehicleState_InverterDirection_T direction)
{
  (void)vc;
  (void)torque;
  (void)direction;
  // TODO
  return VEHICLECONTORL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_SetPowerChannel(VehicleControl_T* vc, uint8_t channel, bool enabled)
{
  (void)vc;
  (void)channel;
  (void)enabled;
  // TODO
  return VEHICLECONTORL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_SetECUError(VehicleControl_T* vc, bool error)
{
  (void)vc;
  (void)error;
  // TODO
  return VEHICLECONTORL_STATUS_OK;
}

VehicleControl_Status_T VehicleControl_SetDash(VehicleControl_T* vc, bool ledOn)
{
  (void)vc;
  (void)ledOn;
  // TODO
  return VEHICLECONTORL_STATUS_OK;
}
