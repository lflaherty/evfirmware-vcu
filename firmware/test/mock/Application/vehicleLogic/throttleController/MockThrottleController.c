/*
 * MockThrottleController.c
 *
 *  Created on: 24 Jun 2022
 *      Author: Liam Flaherty
 */

#include "MockThrottleController.h"

// ------------------- Static data -------------------
static ThrottleController_Status_T mStatus_ThrottleController_Init = THROTTLECONTROLLER_STATUS_OK;
static ThrottleController_Status_T mStatus_ThrottleController_SetTorqueEnabled = THROTTLECONTROLLER_STATUS_OK;
static ThrottleController_Status_T mStatus_ThrottleController_SetMotorDirection = THROTTLECONTROLLER_STATUS_OK;
static bool mLatestTorqueEnabled = false;
static VehicleState_InverterDirection_T mLatestMotorDirection = VEHICLESTATE_INVERTER_FORWARD;

// ------------------- Methods -------------------
ThrottleController_Status_T stub_ThrottleController_Init(
    Logging_T* logger,
    ThrottleController_T* throttleControl)
{
    (void)logger;
    (void)throttleControl;
    return mStatus_ThrottleController_Init;
}

ThrottleController_Status_T stub_ThrottleController_SetTorqueEnabled(
    ThrottleController_T* throttleControl,
    bool enabled)
{
    (void)throttleControl;

    mLatestTorqueEnabled = enabled;

    return mStatus_ThrottleController_SetTorqueEnabled;
}

ThrottleController_Status_T stub_ThrottleController_SetMotorDirection(
    ThrottleController_T* throttleControl,
    VehicleState_InverterDirection_T direction)
{
    (void)throttleControl;

    mLatestMotorDirection = direction;

    return mStatus_ThrottleController_SetMotorDirection;
}

void mockSet_ThrottleController_Init_Status(ThrottleController_Status_T status)
{
    mStatus_ThrottleController_Init = status;
}

void mockSet_ThrottleController_SetTorqueEnabled_Status(ThrottleController_Status_T status)
{
    mStatus_ThrottleController_SetTorqueEnabled = status;
}

void mockSet_ThrottleController_SetMotorDirection_Status(ThrottleController_Status_T status)
{
    mStatus_ThrottleController_SetMotorDirection = status;
}

bool mockGet_ThrottleController_TorqueEnable(void)
{
    return mLatestTorqueEnabled;
}

void mockSet_ThrottleController_TorqueEnable(bool enabled)
{
    mLatestTorqueEnabled = enabled;
}

VehicleState_InverterDirection_T mockGet_ThrottleController_MotorDirection(void)
{
    return mLatestMotorDirection;
}

void mockSet_ThrottleController_MotorDirection(VehicleState_InverterDirection_T direction)
{
    mLatestMotorDirection = direction;
}
