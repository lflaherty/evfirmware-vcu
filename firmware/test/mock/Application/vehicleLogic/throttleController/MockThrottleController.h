/*
 * MockThrottleController.h
 *
 *  Created on: 24 Jun 2022
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_VEHICLELOGIC_THROTTLECONTROLLER_THROTTLECONTROLLER_H_
#define _MOCK_VEHICLELOGIC_THROTTLECONTROLLER_THROTTLECONTROLLER_H_

// Redefine methods to be mocked
#define ThrottleController_Init stub_ThrottleController_Init
#define ThrottleController_SetTorqueEnabled stub_ThrottleController_SetTorqueEnabled
#define ThrottleController_SetMotorDirection stub_ThrottleController_SetMotorDirection

// Bring in the header to be mocked
#include "vehicleLogic/throttleController/throttleController.h"

// ============= Mock control methods =============
void mockSet_ThrottleController_Init_Status(ThrottleController_Status_T status);
void mockSet_ThrottleController_SetTorqueEnabled_Status(ThrottleController_Status_T status);
void mockSet_ThrottleController_SetMotorDirection_Status(ThrottleController_Status_T status);

bool mockGet_ThrottleController_TorqueEnable(void);
void mockSet_ThrottleController_TorqueEnable(bool enabled);
VehicleState_InverterDirection_T mockGet_ThrottleController_MotorDirection(void);
void mockSet_ThrottleController_MotorDirection(VehicleState_InverterDirection_T direction);

#endif // _MOCK_VEHICLELOGIC_THROTTLECONTROLLER_THROTTLECONTROLLER_H_
