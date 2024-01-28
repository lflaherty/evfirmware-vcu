/*
 * MockStateMachine.h
 *
 *  Created on: 19 Jun 2022
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_VEHICLELOGIC_STATEMANAGER_STATEMACHINE_H_
#define _MOCK_VEHICLELOGIC_STATEMANAGER_STATEMACHINE_H_

#include <stdint.h>

// Redefine methods to be mocked
#define VSM_Init stub_VSM_Init
#define VSM_Step stub_VSM_Step

// Bring in the header to be mocked
#include "vehicleLogic/stateManager/stateMachine.h"

// ============= Mock control methods =============
/**
 * @brief Returns the number of times VSM_Step has been called
 * 
 * @return uint32_t Number of invocations
 */
uint32_t mockGet_VSM_Step_Count(void);

/**
 * @brief Resets the recorded number of VSM steps to 0.
 * 
 */
void mockReset_VSM_Step_Count(void);

#endif // _MOCK_VEHICLELOGIC_STATEMANAGER_STATEMACHINE_H_
