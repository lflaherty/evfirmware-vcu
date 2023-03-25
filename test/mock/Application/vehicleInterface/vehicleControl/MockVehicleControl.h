/*
 * MockVehicleControl.h
 *
 *  Created on: 22 Jun 2022
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_VEHICLEINTERFACE_VEHICLECONTROL_VEHICLECONTROL_H_
#define _MOCK_VEHICLEINTERFACE_VEHICLECONTROL_VEHICLECONTROL_H_

#include <stdint.h>

// Also need to mock out the devices that VehicleControl depends on
#include "device/inverter/MockCInverter.h"

// Redefine methods to be mocked
#define VehicleControl_Init stub_VehicleControl_Init
#define VehicleControl_EnableInverter stub_VehicleControl_EnableInverter
#define VehicleControl_DisableInverter stub_VehicleControl_DisableInverter
#define VehicleControl_RequestMotorTorque stub_VehicleControl_RequestMotorTorque
#define VehicleControl_SetPowerChannel stub_VehicleControl_SetPowerChannel
#define VehicleControl_SetECUError stub_VehicleControl_SetECUError
#define VehicleControl_SetDash stub_VehicleControl_SetDash

// Bring in the header to be mocked
#include "vehicleInterface/vehicleControl/vehicleControl.h"

// ============= Mock control methods =============
void mockSet_VehicleControl_Init_Status(VehicleControl_Status_T status);
void mockSet_VehicleControl_EnableInverter_Status(VehicleControl_Status_T status);
void mockSet_VehicleControl_DisableInverter_Status(VehicleControl_Status_T status);
void mockSet_VehicleControl_RequestMotorTorque_Status(VehicleControl_Status_T status);
void mockSet_VehicleControl_SetPowerChannel_Status(VehicleControl_Status_T status);
void mockSet_VehicleControl_SetECUError_Status(VehicleControl_Status_T status);
void mockSet_VehicleControl_SetDash_Status(VehicleControl_Status_T status);

void mockReset_VehicleControl(void);
void mockReset_VehicleControl_RequestMotorTorque(void);
uint32_t mockGet_VehicleControl_RequestMotorTorque_NumRequests(void);
void mockSet_VehicleControl_RequestMotorTorque_LastRequest(float torqueRequest);
float mockGet_VehicleControl_RequestMotorTorque_LastRequest(void);
void mockSet_VehicleControl_RequestMotorTorque_LastRequestDir(VehicleState_InverterDirection_T torqueDirection);
VehicleState_InverterDirection_T mockGet_VehicleControl_RequestMotorTorque_LastRequestDir(void);
void mockSet_VehicleControl_ECUError(bool error);
bool mockGet_VehicleControl_ECUError(void);
void mockSet_VehicleControl_PDMChannel(uint8_t channel, bool en);
bool mockGet_VehicleControl_PDMChannel(uint8_t ch);

#endif // _MOCK_VEHICLEINTERFACE_VEHICLECONTROL_VEHICLECONTROL_H_
