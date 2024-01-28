/*
 * MockPdm.h
 *
 *  Created on: 25 Mar 2023
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_DEVICE_PDM_PDM_H_
#define _MOCK_DEVICE_PDM_PDM_H_

// Redefine methods to be mocked
#define PDM_Init stub_PDM_Init
#define PDM_SetOutputEnabled stub_PDM_SetOutputEnabled

// Bring in the header to be mocked
#include "device/pdm/pdm.h"

// ============= Mock control methods =============
void mockSet_PDM_Init_Status(PDM_Status_T status);
void mockSet_PDM_SetOutputEnabled_Status(PDM_Status_T status);

#endif // _MOCK_DEVICE_PDM_PDM_H_
