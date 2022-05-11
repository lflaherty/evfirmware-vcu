/*
 * vehicleControl.c
 *
 *  Created on: May 6 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_VEHICLECONTROL_VEHICLECONTROL_H_
#define VEHICLEINTERFACE_VEHICLECONTROL_VEHICLECONTROL_H_

#include <stdint.h>
#include <stdbool.h>

#include "lib/logging/logging.h"
#include "vehicleInterface/vehicleState/vehicleStateTypes.h"

typedef enum
{
  VEHICLECONTORL_STATUS_OK    = 0x00U
} VehicleControl_Status_T;

typedef struct
{
  // TODO references to objects
} VehicleControl_T;

/**
 * @brief Initialize the vehicle control process
 * @param logger Pointer to system logger
 * @param state Pointer to VehicleControl object
 * @returns Success status. VEHICLECONTORL_STATUS_OK if successful.
 */
VehicleControl_Status_T VehicleControl_Init(Logging_T* logger);

VehicleControl_Status_T VehicleControl_EnableInverter(VehicleControl_T* vc);
VehicleControl_Status_T VehicleControl_DisableInverter(VehicleControl_T* vc);
VehicleControl_Status_T VehicleControl_RequestMotorTorque(VehicleControl_T* vc, float torque, VehicleState_InverterDirection_T direction);
VehicleControl_Status_T VehicleControl_SetPowerChannel(VehicleControl_T* vc, uint8_t channel, bool enabled);
VehicleControl_Status_T VehicleControl_SetECUError(VehicleControl_T* vc, bool error);
VehicleControl_Status_T VehicleControl_SetDash(VehicleControl_T* vc, bool ledOn);


#endif // VEHICLEINTERFACE_VEHICLECONTROL_VEHICLECONTROL_H_ 