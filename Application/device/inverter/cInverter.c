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

#include "dataConversions.h"

// ------------------- Private data -------------------
static Logging_T* mLog;

// ------------------- Private methods -------------------
static void CInverter_Callback_Temperatures1(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_Temperatures1_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float moduleATemp = msgToTemperature(dataView.fields.moduleATemp);
  float moduleBTemp = msgToTemperature(dataView.fields.moduleBTemp);
  float moduleCTemp = msgToTemperature(dataView.fields.moduleCTemp);
  float gateDriverTemp = msgToTemperature(dataView.fields.gateDriverTemp);

  // TODO send to vehicle state
  (void)moduleATemp;
  (void)moduleBTemp;
  (void)moduleCTemp;
  (void)gateDriverTemp;
}

static void CInverter_Callback_Temperatures2(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_Temperatures2_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float controlBoardTemp = msgToTemperature(dataView.fields.controlBoardTemp);

  // TODO send to vehicle state
  (void)controlBoardTemp;
}

static void CInverter_Callback_Temperatures3(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_Temperatures3_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float motorTemp = msgToTemperature(dataView.fields.motorTemp);

  // TODO send to vehicle state
  (void)motorTemp;
}

static void CInverter_Callback_MotorPosInfo(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_MotorPosInfo_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float motorAngle = msgToAngle(dataView.fields.motorAngle);
  int16_t motorSpeed = msgToAngularVelocity(dataView.fields.motorSpeed);
  float electricalOutFreq = msgToFrequency(dataView.fields.electricalOutFreq);

  // TODO send to vehicle state
  (void)motorAngle;
  (void)motorSpeed;
  (void)electricalOutFreq;
}

static void CInverter_Callback_CurrentInfo(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_CurrentInfo_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float phaseACurrent = msgToCurrent(dataView.fields.phaseACurrent);
  float phaseBCurrent = msgToCurrent(dataView.fields.phaseBCurrent);
  float phaseCCurrent = msgToCurrent(dataView.fields.phaseCCurrent);
  float dcBusCurrent = msgToCurrent(dataView.fields.dcBusCurrent);

  // TODO send to vehicle state
  (void)phaseACurrent;
  (void)phaseBCurrent;
  (void)phaseCCurrent;
  (void)dcBusCurrent;
}

static void CInverter_Callback_VoltageInfo(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_VoltageInfo_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float dcBusVoltage = msgToVoltageHigh(dataView.fields.dcBusVoltage);
  float outputVoltage = msgToVoltageHigh(dataView.fields.outputVoltage);
  float vd = msgToVoltageHigh(dataView.fields.vd);
  float vq = msgToVoltageHigh(dataView.fields.vq);

  // TODO send to vehicle state
  (void)dcBusVoltage;
  (void)outputVoltage;
  (void)vd;
  (void)vq;
}

static void CInverter_Callback_FluxInfo(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_FluxInfo_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float fluxCommand = msgToFlux(dataView.fields.fluxCommand);
  float fluxFeedback = msgToFlux(dataView.fields.fluxFeedback);
  float idFeedback = msgToCurrent(dataView.fields.idFeedback);
  float iqFeedback = msgToCurrent(dataView.fields.iqFeedback);

  // TODO send to vehicle state
  (void)fluxCommand;
  (void)fluxFeedback;
  (void)idFeedback;
  (void)iqFeedback;
}

static void CInverter_Callback_InternalStates(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_InternalStates_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  uint16_t inverterState = dataView.fields.inverterState; // TODO convert to enum (enum to be defined in vehicle state)
  uint16_t activeDischargeState = dataView.fields.activeDischargeState; // TODO convert to enum
  bool inverterEnabled = dataView.fields.inverterEnabled;
  uint16_t direction = dataView.fields.direction; // TODO convert to enum

  // TODO send to vehicle state
  (void)inverterState;
  (void)activeDischargeState;
  (void)inverterEnabled;
  (void)direction;
}

static void CInverter_Callback_FaultCodes(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_FaultCodes_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  // TODO process and send to vehicle state (i.e. check that no errors are present)
}

static void CInverter_Callback_TorqueTimer(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_TorqueTimer_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float commandedTorque = msgToTorque(dataView.fields.commandedTorque);
  float feedbackTorque = msgToTorque(dataView.fields.feedbackTorque);
  uint32_t timerMs = dataView.fields.timer;

  // TODO send to vehicle state
  (void)commandedTorque;
  (void)feedbackTorque;
  (void)timerMs;
}

static void CInverter_Callback_FluxWeakening(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_FluxWeakening_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float modulationIndex = msgToModulationIndex(dataView.fields.modulationIndex);
  float fluxWeakeningOutput = msgToCurrent(dataView.fields.fluxWeakeningOutput);
  float idCommand = msgToCurrent(dataView.fields.idCommand);
  float iqCommand = msgToCurrent(dataView.fields.iqCommand);

  // TODO send to vehicle state
  (void)modulationIndex;
  (void)fluxWeakeningOutput;
  (void)idCommand;
  (void)iqCommand;
}

// ------------------- Public methods -------------------
CInverter_Status_T CInverter_Init(Logging_T* logger, CInverter_T* inv)
{
  mLog = logger;
  logPrintS(mLog, "CInverter_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // Register callbacks
  CAN_Status_T callbackRegStatus = CAN_STATUS_OK;
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_TEMPERATURES1, CInverter_Callback_Temperatures1, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_TEMPERATURES2, CInverter_Callback_Temperatures2, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_TEMPERATURES3, CInverter_Callback_Temperatures3, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_MOTOR_POS_INFO, CInverter_Callback_MotorPosInfo, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_CURRENT_INFO, CInverter_Callback_CurrentInfo, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_VOLTAGE_INFO, CInverter_Callback_VoltageInfo, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_FLUX_INFO, CInverter_Callback_FluxInfo, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_INTERNAL_STATES, CInverter_Callback_InternalStates, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_FAULT_CODES, CInverter_Callback_FaultCodes, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_TORQUE_TIMER, CInverter_Callback_TorqueTimer, (void*)inv);
  callbackRegStatus |= CAN_RegisterCallback(inv->hcan, CINVERTER_CAN_ID_FLUX_WEAKENING, CInverter_Callback_FluxWeakening, (void*)inv);

  if (CAN_STATUS_OK != callbackRegStatus) {
    return CINVERTER_STATUS_ERROR_CAN;
  }

  logPrintS(mLog, "CInverter_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return CINVERTER_STATUS_OK;
}
