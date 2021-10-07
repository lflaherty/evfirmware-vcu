/*
 * cInverter.c
 *
 *  Created on: Oct 7 2021
 *      Author: Liam Flaherty
 */

#include "cInverter.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f7xx_hal.h"

#include "lib/logging/logging.h"
#include "comm/can/can.h"

// ------------------- Private data -------------------
static Logging_T* mLog;

// ------------------- Private methods -------------------
static void CInverter_Callback_Temperatures1(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_Temperatures2(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_Temperatures3(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_MotorPosInfo(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_CurrentInfo(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_VoltageInfo(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_FluxInfo(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_InternalStates(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_FaultCodes(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_TorqueTimer(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

static void CInverter_Callback_FluxWeakening(const CAN_DataFrame_T* data, const void* param)
{
  // TODO
}

// ------------------- Public methods -------------------
CInverter_Status_T CInverter_Init(Logging_T* logger, CInverter_T* inv)
{
  mLog = logger;
  logPrintS(mLog, "CInverter_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // Register callbacks
  CAN_Status_T callbackRegStatus = CAN_STATUS_OK;
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_TEMPERATURES1, CInverter_Callback_Temperatures1, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_TEMPERATURES2, CInverter_Callback_Temperatures2, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_TEMPERATURES3, CInverter_Callback_Temperatures3, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_MOTOR_POS_INFO, CInverter_Callback_MotorPosInfo, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_CURRENT_INFO, CInverter_Callback_CurrentInfo, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_VOLTAGE_INFO, CInverter_Callback_VoltageInfo, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_FLUX_INFO, CInverter_Callback_FluxInfo, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_INTERNAL_STATES, CInverter_Callback_InternalStates, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_FAULT_CODES, CInverter_Callback_FaultCodes, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_TORQUE_TIMER, CInverter_Callback_TorqueTimer, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_FLUX_WEAKENING, CInverter_Callback_FluxWeakening, (void*)inv);

  if (CAN_STATUS_OK != callbackRegStatus) {
    return CINVERTER_STATUS_ERROR_CAN;
  }

  logPrintS(mLog, "CInverter_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return CINVERTER_STATUS_OK;
}
