/*
 * MockPdm.c
 *
 *  Created on: 25 Mar 2023
 *      Author: Liam Flaherty
 */

#include "MockPdm.h"

// ------------------- Static data -------------------
static PDM_Status_T mStatus_PDM_Init = PDM_STATUS_OK;
static PDM_Status_T mStatus_PDM_SetOutputEnabled = PDM_STATUS_OK;

// ------------------- Methods -------------------
PDM_Status_T stub_PDM_Init(Logging_T* logger, PDM_T* pdm)
{
    (void)logger;
    REGISTER(pdm, PDM_STATUS_ERROR_DEPENDS);
    return mStatus_PDM_Init;
}

PDM_Status_T stub_PDM_SetOutputEnabled(
    PDM_T* pdm,
    uint8_t channel,
    bool state)
{
    (void)pdm;
    (void)channel;
    (void)state;
    return mStatus_PDM_SetOutputEnabled;
}

void mockSet_PDM_Init_Status(PDM_Status_T status)
{
    mStatus_PDM_Init = status;
}

void mockSet_PDM_SetOutputEnabled_Status(PDM_Status_T status)
{
    mStatus_PDM_SetOutputEnabled = status;
}
