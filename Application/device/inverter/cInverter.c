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

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushFieldf(state, &state->data.inverter.moduleATemperature, moduleATemp);
  VehicleState_PushFieldf(state, &state->data.inverter.moduleBTemperature, moduleBTemp);
  VehicleState_PushFieldf(state, &state->data.inverter.moduleCTemperature, moduleCTemp);
  VehicleState_PushFieldf(state, &state->data.inverter.gateDriverTemp, gateDriverTemp);
}

static void CInverter_Callback_Temperatures2(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_Temperatures2_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float controlBoardTemp = msgToTemperature(dataView.fields.controlBoardTemp);

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushFieldf(state, &state->data.inverter.controlBoardTemp, controlBoardTemp);
}

static void CInverter_Callback_Temperatures3(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_Temperatures3_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float motorTemp = msgToTemperature(dataView.fields.motorTemp);

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushFieldf(state, &state->data.motor.temperature, motorTemp);
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

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushFieldf(state, &state->data.motor.angle, motorAngle);
  VehicleState_PushFieldf(state, &state->data.motor.speed, motorSpeed);
  VehicleState_PushFieldf(state, &state->data.inverter.outputFrequency, electricalOutFreq);
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

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushFieldf(state, &state->data.motor.phaseACurrent, phaseACurrent);
  VehicleState_PushFieldf(state, &state->data.motor.phaseBCurrent, phaseBCurrent);
  VehicleState_PushFieldf(state, &state->data.motor.phaseCCurrent, phaseCCurrent);
  VehicleState_PushFieldf(state, &state->data.inverter.dcBusCurrent, dcBusCurrent);
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

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushFieldf(state, &state->data.inverter.dcBusVoltage, dcBusVoltage);
  VehicleState_PushFieldf(state, &state->data.inverter.outputVoltage, outputVoltage);
  VehicleState_PushFieldf(state, &state->data.inverter.vd, vd);
  VehicleState_PushFieldf(state, &state->data.inverter.vq, vq);
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

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushFieldf(state, &state->data.inverter.fluxCommand, fluxCommand);
  VehicleState_PushFieldf(state, &state->data.inverter.fluxFeedback, fluxFeedback);
  VehicleState_PushFieldf(state, &state->data.inverter.idFeedback, idFeedback);
  VehicleState_PushFieldf(state, &state->data.inverter.idFeedback, iqFeedback);
}

static void CInverter_Callback_InternalStates(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_InternalStates_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  VehicleState_InverterState_T inverterState = (VehicleState_InverterState_T)dataView.fields.inverterState;
  VehicleState_InverterDischargeState_T activeDischargeState = (VehicleState_InverterDischargeState_T)dataView.fields.activeDischargeState;
  VehicleState_InverterEnabled_T inverterEnabled = (VehicleState_InverterEnabled_T)dataView.fields.inverterEnabled;
  VehicleState_InverterDirection_T direction = (VehicleState_InverterDirection_T)dataView.fields.direction;

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushField(state, &state->data.inverter.state, &inverterState, sizeof(inverterState));
  VehicleState_PushField(state, &state->data.inverter.dischargeState, &activeDischargeState, sizeof(activeDischargeState));
  VehicleState_PushField(state, &state->data.inverter.enabled, &inverterEnabled, sizeof(inverterEnabled));
  VehicleState_PushField(state, &state->data.inverter.direction, &direction, sizeof(direction));
}

static void CInverter_Callback_FaultCodes(const CAN_DataFrame_T* data, const void* param)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_FaultCodes_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushField(state, &state->data.inverter.postFaults, &dataView.fields.postFault, sizeof(uint32_t));
  VehicleState_PushField(state, &state->data.inverter.runFaults, &dataView.fields.runFault, sizeof(uint32_t));
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

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushFieldf(state, &state->data.inverter.commandedTorque, commandedTorque);
  VehicleState_PushFieldf(state, &state->data.motor.calculatedTorque, feedbackTorque);
  VehicleState_PushField(state, &state->data.inverter.timer, &timerMs, sizeof(timerMs));
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

  // send to vehicle state
  VehicleState_T* state = ((CInverter_T*)param)->vehicleState;
  VehicleState_PushFieldf(state, &state->data.inverter.modulationIndex, modulationIndex);
  VehicleState_PushFieldf(state, &state->data.inverter.fluxWeakeningOutput, fluxWeakeningOutput);
  VehicleState_PushFieldf(state, &state->data.inverter.idCommand, idCommand);
  VehicleState_PushFieldf(state, &state->data.inverter.iqCommand, iqCommand);
}

// ------------------- Public methods -------------------
CInverter_Status_T CInverter_Init(Logging_T* logger, CInverter_T* inv)
{
  mLog = logger;
  Log_Print(mLog, "CInverter_Init begin\n");

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

  Log_Print(mLog, "CInverter_Init complete\n");
  return CINVERTER_STATUS_OK;
}
