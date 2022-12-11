/*
 * MockVehicleControl.c
 *
 *  Created on: 22 Jun 2022
 *      Author: Liam Flaherty
 */

#include "MockVehicleControl.h"

// ------------------- Static data -------------------
static VehicleControl_Status_T mStatus_VehicleControl_Init = VEHICLECONTROL_STATUS_OK;
static VehicleControl_Status_T mStatus_VehicleControl_EnableInverter = VEHICLECONTROL_STATUS_OK;
static VehicleControl_Status_T mStatus_VehicleControl_DisableInverter = VEHICLECONTROL_STATUS_OK;
static VehicleControl_Status_T mStatus_VehicleControl_RequestMotorTorque = VEHICLECONTROL_STATUS_OK;
static VehicleControl_Status_T mStatus_VehicleControl_SetPowerChannel = VEHICLECONTROL_STATUS_OK;
static VehicleControl_Status_T mStatus_VehicleControl_SetECUError = VEHICLECONTROL_STATUS_OK;
static VehicleControl_Status_T mStatus_VehicleControl_SetDash = VEHICLECONTROL_STATUS_OK;

static bool mECUError = false;
static uint32_t mRequestMotorTorqueRequests = 0U;
static float mRequestMotorTorqueLatest = 0.0f;
static VehicleState_InverterDirection_T mRequestMotorTorqueLatestDir = VEHICLESTATE_INVERTER_REVERSE;

// ------------------- Methods -------------------
VehicleControl_Status_T stub_VehicleControl_Init(Logging_T* logger, VehicleControl_T* vc)
{
    (void)logger;
    (void)vc;
    REGISTER(vc, VEHICLECONTROL_STATUS_ERROR_DEPENDS);
    return mStatus_VehicleControl_Init;
}

VehicleControl_Status_T stub_VehicleControl_EnableInverter(VehicleControl_T* vc)
{
    (void)vc;
    return mStatus_VehicleControl_EnableInverter;
}

VehicleControl_Status_T stub_VehicleControl_DisableInverter(VehicleControl_T* vc)
{
    (void)vc;
    return mStatus_VehicleControl_DisableInverter;
}

VehicleControl_Status_T stub_VehicleControl_RequestMotorTorque(VehicleControl_T* vc, float torque, VehicleState_InverterDirection_T direction)
{
    (void)vc;
    mRequestMotorTorqueRequests++;
    mRequestMotorTorqueLatest = torque;
    mRequestMotorTorqueLatestDir = direction;
    return mStatus_VehicleControl_RequestMotorTorque;
}

VehicleControl_Status_T stub_VehicleControl_SetPowerChannel(VehicleControl_T* vc, uint8_t channel, bool enabled)
{
    (void)vc;
    (void)channel;
    (void)enabled;
    return mStatus_VehicleControl_SetPowerChannel;
}

VehicleControl_Status_T stub_VehicleControl_SetECUError(VehicleControl_T* vc, bool error)
{
    (void)vc;
    mECUError = error;
    return mStatus_VehicleControl_SetECUError;
}

VehicleControl_Status_T stub_VehicleControl_SetDash(VehicleControl_T* vc, bool ledOn)
{
    (void)vc;
    (void)ledOn;
    return mStatus_VehicleControl_SetDash;
}

void mockSet_VehicleControl_Init_Status(VehicleControl_Status_T status)
{
    mStatus_VehicleControl_Init = status;
}

void mockSet_VehicleControl_EnableInverter_Status(VehicleControl_Status_T status)
{
    mStatus_VehicleControl_EnableInverter = status;
}

void mockSet_VehicleControl_DisableInverter_Status(VehicleControl_Status_T status)
{
    mStatus_VehicleControl_DisableInverter = status;
}

void mockSet_VehicleControl_RequestMotorTorque_Status(VehicleControl_Status_T status)
{
    mStatus_VehicleControl_RequestMotorTorque = status;
}

void mockSet_VehicleControl_SetPowerChannel_Status(VehicleControl_Status_T status)
{
    mStatus_VehicleControl_SetPowerChannel = status;
}

void mockSet_VehicleControl_SetECUError_Status(VehicleControl_Status_T status)
{
    mStatus_VehicleControl_SetECUError = status;
}

void mockSet_VehicleControl_SetDash_Status(VehicleControl_Status_T status)
{
    mStatus_VehicleControl_SetDash = status;
}

void mockReset_VehicleControl(void)
{
    mockReset_VehicleControl_RequestMotorTorque();
    mockSet_VehicleControl_RequestMotorTorque_LastRequest(0.0f);
    mockSet_VehicleControl_RequestMotorTorque_LastRequestDir(VEHICLESTATE_INVERTER_FORWARD);
    mockSet_VehicleControl_ECUError(false);
}

void mockReset_VehicleControl_RequestMotorTorque(void)
{
    mRequestMotorTorqueRequests = 0U;
}

uint32_t mockGet_VehicleControl_RequestMotorTorque_NumRequests(void)
{
    return mRequestMotorTorqueRequests;
}

void mockSet_VehicleControl_RequestMotorTorque_LastRequest(float torqueRequest)
{
    mRequestMotorTorqueLatest = torqueRequest;
}

float mockGet_VehicleControl_RequestMotorTorque_LastRequest(void)
{
    return mRequestMotorTorqueLatest;
}

void mockSet_VehicleControl_RequestMotorTorque_LastRequestDir(VehicleState_InverterDirection_T torqueDirection)
{
    mRequestMotorTorqueLatestDir = torqueDirection;
}

VehicleState_InverterDirection_T mockGet_VehicleControl_RequestMotorTorque_LastRequestDir(void)
{
    return mRequestMotorTorqueLatestDir;
}

void mockSet_VehicleControl_ECUError(bool error)
{
    mECUError = error;
}

bool mockGet_VehicleControl_ECUError(void)
{
    return mECUError;
}
