/*
 * MockCInverter.c
 *
 *  Created on: 25 Mar 2023
 *      Author: Liam Flaherty
 */

#include "MockCInverter.h"

// ------------------- Static data -------------------
static CInverter_Status_T mStatus_CInverter_Init = CINVERTER_STATUS_OK;
static CInverter_Status_T mStatus_CInverter_SendTorqueCommand = CINVERTER_STATUS_OK;
static CInverter_Status_T mStatus_CInverter_SendInverterEnabled = CINVERTER_STATUS_OK;
static CInverter_Status_T mStatus_CInverter_SendInverterDischarge = CINVERTER_STATUS_OK;

// ------------------- Methods -------------------
CInverter_Status_T stub_CInverter_Init(Logging_T* logger, CInverter_T* inv)
{
    (void)logger;
    REGISTER(inv, CINVERTER_STATUS_ERROR_DEPENDS);
    return mStatus_CInverter_Init;
}

CInverter_Status_T stub_CInverter_SendTorqueCommand(CInverter_T* inv, const float torqueNm, const VehicleState_InverterDirection_T direction)
{
    (void)inv;
    (void)torqueNm;
    (void)direction;
    return mStatus_CInverter_SendTorqueCommand;
}

CInverter_Status_T stub_CInverter_SendInverterEnabled(CInverter_T* inv, const bool enabled)
{
    (void)inv;
    (void)enabled;
    return mStatus_CInverter_SendInverterEnabled;
}

CInverter_Status_T stub_CInverter_SendInverterDischarge(CInverter_T* inv, const bool dischargeModeEnabled)
{
    (void)inv;
    (void)dischargeModeEnabled;
    return mStatus_CInverter_SendInverterDischarge;
}

void mockSet_CInverter_Init_Status(CInverter_Status_T status)
{
    mStatus_CInverter_Init = status;
}

void mockSet_CInverter_SendTorqueCommand_Status(CInverter_Status_T status)
{
    mStatus_CInverter_SendTorqueCommand = status;
}

void mockSet_CInverter_SendInverterEnabled_Status(CInverter_Status_T status)
{
    mStatus_CInverter_SendInverterEnabled = status;
}

void mockSet_CInverter_SendInverterDischarge_Status(CInverter_Status_T status)
{
    mStatus_CInverter_SendInverterDischarge = status;
}
