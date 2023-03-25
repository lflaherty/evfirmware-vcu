/*
 * MockCInverter.h
 *
 *  Created on: 25 Mar 2023
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_DEVICE_INVERTER_CINVERTER_H_
#define _MOCK_DEVICE_INVERTER_CINVERTER_H_

// Redefine methods to be mocked
#define CInverter_Init stub_CInverter_Init
#define CInverter_SendTorqueCommand stub_CInverter_SendTorqueCommand
#define CInverter_SendInverterEnabled stub_CInverter_SendInverterEnabled
#define CInverter_SendInverterDischarge stub_CInverter_SendInverterDischarge

// Bring in the header to be mocked
#include "device/inverter/cInverter.h"

// ============= Mock control methods =============
void mockSet_CInverter_Init_Status(CInverter_Status_T status);
void mockSet_CInverter_SendTorqueCommand_Status(CInverter_Status_T status);
void mockSet_CInverter_SendInverterEnabled_Status(CInverter_Status_T status);
void mockSet_CInverter_SendInverterDischarge_Status(CInverter_Status_T status);

#endif // _MOCK_DEVICE_INVERTER_CINVERTER_H_
