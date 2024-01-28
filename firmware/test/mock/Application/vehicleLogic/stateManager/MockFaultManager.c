/*
 * MockFaultManager.c
 *
 *  Created on: 14 May 2022
 *      Author: Liam Flaherty
 */

#include "MockFaultManager.h"

// ------------------- Static data -------------------
static FaultStatus_T mStatus_FaultManager_Step = FAULT_NO_FAULT;

// ------------------- Methods -------------------
void stub_FaultManager_Init(Logging_T* logger, FaultManager_T* faultMgr)
{
    (void)logger;
    (void)faultMgr;
}

FaultStatus_T stub_FaultManager_Step(FaultManager_T* faultMgr)
{
    (void)faultMgr;
    return mStatus_FaultManager_Step;
}

void mockSet_FaultManager_Step_Status(FaultStatus_T status)
{
    mStatus_FaultManager_Step = status;
}
