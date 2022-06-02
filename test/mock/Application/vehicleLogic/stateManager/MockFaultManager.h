/*
 * MockFaultManager.h
 *
 *  Created on: 14 May 2022
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_VEHICLELOGIC_STATEMANAGER_FAULT_H_
#define _MOCK_VEHICLELOGIC_STATEMANAGER_FAULT_H_

// Redefine methods to be mocked
#define FaultManager_Init stub_FaultManager_Init
#define FaultManager_Step stub_FaultManager_Step

// Bring in the header to be mocked
#include "vehicleLogic/stateManager/faultManager.h"

// ============= Mock control methods =============
void mockSet_FaultManager_Step_Status(FaultStatus_T status);

#endif // _MOCK_VEHICLELOGIC_STATEMANAGER_FAULT_H_