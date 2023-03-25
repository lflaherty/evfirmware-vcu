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
#include "lib/depends/depends.h"
#include "vehicleInterface/vehicleState/vehicleStateTypes.h"

#include "device/inverter/cInverter.h"
#include "device/sdc/sdc.h"

typedef enum
{
  VEHICLECONTROL_STATUS_OK            = 0x00U,
  VEHICLECONTROL_STATUS_ERROR_DEPENDS = 0x01U,
  VEHICLECONTROL_STATUS_ERROR_DRIVER  = 0x02U,
} VehicleControl_Status_T;

typedef struct
{
  // Pointers to modules required to control vehicle:
  CInverter_T* inverter;
  REGISTERED_MODULE();
} VehicleControl_T;

/**
 * @brief Initialize the vehicle control process
 * @param logger Pointer to system logger
 * @param control VehicleControl struct
 * @returns Success status. VEHICLECONTROL_STATUS_OK if successful.
 */
VehicleControl_Status_T VehicleControl_Init(
    Logging_T* logger,
    VehicleControl_T* control);

VehicleControl_Status_T VehicleControl_EnableInverter(VehicleControl_T* vc);
VehicleControl_Status_T VehicleControl_DisableInverter(VehicleControl_T* vc);
VehicleControl_Status_T VehicleControl_RequestMotorTorque(VehicleControl_T* vc, float torque, VehicleState_InverterDirection_T direction);
VehicleControl_Status_T VehicleControl_SetPowerChannel(VehicleControl_T* vc, uint8_t channel, bool enabled);
VehicleControl_Status_T VehicleControl_SetECUError(VehicleControl_T* vc, bool error);
VehicleControl_Status_T VehicleControl_SetDash(VehicleControl_T* vc, bool ledOn);


#endif // VEHICLEINTERFACE_VEHICLECONTROL_VEHICLECONTROL_H_
