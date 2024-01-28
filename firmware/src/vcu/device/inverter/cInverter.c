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
#include "time/tasktimer/tasktimer.h"

#include "dataConversions.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms

// ------------------- Private methods -------------------
static bool SendCANCommand(
    const CAN_Device_T canInstance,
    const float torqueNm,
    const VehicleState_InverterDirection_T direction,
    const bool inverterEnable,
    const bool dischargeEnable)
{
  CInverter_CAN_Command_T dataView = { 0 };

  dataView.fields.torqueNm = torqueToMsg(torqueNm);
  dataView.fields.speed = 0; // speed mode unused
  dataView.fields.directionCommand = direction & 0x1;
  dataView.fields.inverterEnable = inverterEnable & 0x1;
  dataView.fields.inverterDischarge = dischargeEnable & 0x1;
  dataView.fields.speedModeEnabled = 0; // speed mode never used
  dataView.fields.torqueLim = 0; // use EEPROM params for limits

  CAN_Status_T canStatus = CAN_SendMessage(
    canInstance,
    CINVERTER_CAN_ID_COMMAND,
    dataView.raw,
    8);

  return CAN_STATUS_OK == canStatus;
}

static void HandleMsg_Temperatures1(const CAN_DataFrame_T* data, VehicleState_T* state)
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
  if (VehicleState_AccessAcquire(state)) {
    state->data.inverter.moduleATemperature = moduleATemp;
    state->data.inverter.moduleBTemperature = moduleBTemp;
    state->data.inverter.moduleCTemperature = moduleCTemp;
    state->data.inverter.gateDriverTemp = gateDriverTemp;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_Temperatures2(const CAN_DataFrame_T* data, VehicleState_T* state)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_Temperatures2_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float controlBoardTemp = msgToTemperature(dataView.fields.controlBoardTemp);

  // send to vehicle state
  if (VehicleState_AccessAcquire(state)) {
    state->data.inverter.controlBoardTemp = controlBoardTemp;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_Temperatures3(const CAN_DataFrame_T* data, VehicleState_T* state)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_Temperatures3_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float motorTemp = msgToTemperature(dataView.fields.motorTemp);

  // send to vehicle state
  if (VehicleState_AccessAcquire(state)) {
    state->data.motor.temperature = motorTemp;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_MotorPosInfo(const CAN_DataFrame_T* data, VehicleState_T* state)
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
  if (VehicleState_AccessAcquire(state)) {
    state->data.motor.angle = motorAngle;
    state->data.motor.speed = motorSpeed;
    state->data.inverter.outputFrequency = electricalOutFreq;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_CurrentInfo(const CAN_DataFrame_T* data, VehicleState_T* state)
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
  if (VehicleState_AccessAcquire(state)) {
    state->data.motor.phaseACurrent = phaseACurrent;
    state->data.motor.phaseBCurrent = phaseBCurrent;
    state->data.motor.phaseCCurrent = phaseCCurrent;
    state->data.inverter.dcBusCurrent = dcBusCurrent;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_VoltageInfo(const CAN_DataFrame_T* data, VehicleState_T* state)
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
  if (VehicleState_AccessAcquire(state)) {
    state->data.inverter.dcBusVoltage = dcBusVoltage;
    state->data.inverter.outputVoltage = outputVoltage;
    state->data.inverter.vd = vd;
    state->data.inverter.vq = vq;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_FluxInfo(const CAN_DataFrame_T* data, VehicleState_T* state)
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
  if (VehicleState_AccessAcquire(state)) {
    state->data.inverter.fluxCommand = fluxCommand;
    state->data.inverter.fluxFeedback = fluxFeedback;
    state->data.inverter.idFeedback = idFeedback;
    state->data.inverter.iqFeedback = iqFeedback;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_InternalStates(const CAN_DataFrame_T* data, VehicleState_T* state)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_InternalStates_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  VehicleState_InverterVSMState_T inverterVSMState = (VehicleState_InverterVSMState_T)dataView.fields.vsmState;
  VehicleState_InverterState_T inverterState = (VehicleState_InverterState_T)dataView.fields.inverterState;
  VehicleState_InverterDischargeState_T activeDischargeState = (VehicleState_InverterDischargeState_T)dataView.fields.activeDischargeState;
  VehicleState_InverterEnabled_T inverterEnabled = (VehicleState_InverterEnabled_T)dataView.fields.inverterEnabled;
  VehicleState_InverterDirection_T direction = (VehicleState_InverterDirection_T)dataView.fields.direction;

  // send to vehicle state
  if (VehicleState_AccessAcquire(state)) {
    state->data.inverter.vsmState = inverterVSMState;
    state->data.inverter.inverterState = inverterState;
    state->data.inverter.dischargeState = activeDischargeState;
    state->data.inverter.enabled = inverterEnabled;
    state->data.inverter.direction = direction;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_FaultCodes(const CAN_DataFrame_T* data, VehicleState_T* state)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_FaultCodes_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  // send to vehicle state
  if (VehicleState_AccessAcquire(state)) {
    state->data.inverter.postFaults = dataView.fields.postFault;
    state->data.inverter.runFaults = dataView.fields.runFault;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_TorqueTimer(const CAN_DataFrame_T* data, VehicleState_T* state)
{
  if (data->dlc != 8) {
    return;
  }

  CInverter_CAN_TorqueTimer_T dataView;
  memcpy(dataView.raw, data->data, 8*sizeof(uint8_t));

  float commandedTorque = msgToTorque(dataView.fields.commandedTorque);
  float feedbackTorque = msgToTorque(dataView.fields.feedbackTorque);
  uint32_t timerCounts = dataView.fields.timer;

  // send to vehicle state
  if (VehicleState_AccessAcquire(state)) {
    state->data.inverter.commandedTorque = commandedTorque;
    state->data.motor.calculatedTorque = feedbackTorque;
    state->data.inverter.timerCounts = timerCounts;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_FluxWeakening(const CAN_DataFrame_T* data, VehicleState_T* state)
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
  if (VehicleState_AccessAcquire(state)) {
    state->data.inverter.modulationIndex = modulationIndex;
    state->data.inverter.fluxWeakeningOutput = fluxWeakeningOutput;
    state->data.inverter.idCommand = idCommand;
    state->data.inverter.iqCommand = iqCommand;
  }
  VehicleState_AccessRelease(state);
}

static void InverterProcessing(CInverter_T* inv)
{
  // Wait for 10ms notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    CAN_DataFrame_T queuedData;
    while (pdTRUE == xQueueReceive(inv->canDataQueueHandle, &queuedData, 0U)) {
      if (queuedData.busInstance != inv->canInst) {
        // Wrong bus, throw away
        continue;
      }

      switch (queuedData.msgId) {
        case CINVERTER_CAN_ID_TEMPERATURES1:
          HandleMsg_Temperatures1(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_TEMPERATURES2:
          HandleMsg_Temperatures2(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_TEMPERATURES3:
          HandleMsg_Temperatures3(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_MOTOR_POS_INFO:
          HandleMsg_MotorPosInfo(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_CURRENT_INFO:
          HandleMsg_CurrentInfo(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_VOLTAGE_INFO:
          HandleMsg_VoltageInfo(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_FLUX_INFO:
          HandleMsg_FluxInfo(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_INTERNAL_STATES:
          HandleMsg_InternalStates(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_FAULT_CODES:
          HandleMsg_FaultCodes(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_TORQUE_TIMER:
          HandleMsg_TorqueTimer(&queuedData, inv->vehicleState);
          break;
        case CINVERTER_CAN_ID_FLUX_WEAKENING:
          HandleMsg_FluxWeakening(&queuedData, inv->vehicleState);
          break;
        default:
          // Don't know this message ID - throw away message
          break;
      }
    }
  }

}

// LCOV_EXCL_START
static void InverterProcessing_Task(void* pvParameters)
{
  CInverter_T* inv = (CInverter_T*)pvParameters;

  while (1) {
    InverterProcessing(inv);
  }
}
// LCOV_EXCL_STOP

// ------------------- Public methods -------------------
CInverter_Status_T CInverter_Init(Logging_T* logger, CInverter_T* inv)
{
  mLog = logger;
  Log_Print(mLog, "CInverter_Init begin\n");
  DEPEND_ON(logger, CINVERTER_STATUS_ERROR_DEPENDS);
  DEPEND_ON_STATIC(CAN, CINVERTER_STATUS_ERROR_DEPENDS);

  // Create queue for receiving CAN data
  inv->canDataQueueHandle = xQueueCreateStatic(
      INVERTER_QUEUE_LENGTH,
      INVERTER_QUEUE_DATA_SIZE,
      inv->canDataQueueStorageArea,
      &inv->canDataQueueBuffer);

  // Init command data storage
  memset(&inv->commandData, 0, sizeof(struct CInverterCommand));
  inv->commandData.mutex = xSemaphoreCreateMutexStatic(&inv->commandData.mutexBuffer);

  // Create RTOS task
  inv->taskHandle = xTaskCreateStatic(
      InverterProcessing_Task,
      "CInverter",
      INVERTER_STACK_SIZE,   /* Stack size */
      (void*)inv,  /* Parameter passed as pointer */
      INVERTER_TASK_PRIORITY,
      inv->taskStack,
      &inv->taskBuffer);

  // Register RTOS task for 100Hz updates
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&inv->taskHandle, TASKTIMER_FREQUENCY_100HZ);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return CINVERTER_STATUS_ERROR_INIT;
  }

  // Start receiving CAN data
  CAN_Status_T callbackRegStatus = CAN_RegisterQueue(
      inv->canInst,
      INVERTER_CAN_DEVICEID,
      INVERTER_CAN_DEVICEIDMASK,
      inv->canDataQueueHandle);
  if (CAN_STATUS_OK != callbackRegStatus) {
    return CINVERTER_STATUS_ERROR_CAN;
  }

  REGISTER(inv, CINVERTER_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "CInverter_Init complete\n");
  return CINVERTER_STATUS_OK;
}

CInverter_Status_T CInverter_SendTorqueCommand(
    CInverter_T* inv,
    float torqueNm,
    VehicleState_InverterDirection_T direction)
{
  if (torqueNm < 0.0f || torqueNm > INVERTER_MAX_TORQUE) {
    return CINVERTER_STATUS_ERROR_VALUE;
  }

  BaseType_t result = xSemaphoreTake(inv->commandData.mutex, portMAX_DELAY);
  if (pdTRUE != result) {
    return CINVERTER_STATUS_ERROR_LOCK;
  }

  // state machine needs to enable the inverter first
  if (!inv->commandData.inverterEnabled) {
    xSemaphoreGive(inv->commandData.mutex);
    return CINVERTER_STATUS_ERROR_NOT_ENABLED;
  }

  // send normal command message
  bool canSucc = SendCANCommand(
    inv->canInst,
    torqueNm,
    direction,
    true, // inverter enable
    false // discharge disable
  );

  xSemaphoreGive(inv->commandData.mutex);
  return canSucc ? CINVERTER_STATUS_OK : CINVERTER_STATUS_ERROR_CAN;
}

CInverter_Status_T CInverter_SendInverterEnabled(CInverter_T* inv, bool enabled)
{
  BaseType_t result = xSemaphoreTake(inv->commandData.mutex, portMAX_DELAY);
  if (pdTRUE != result) {
    return CINVERTER_STATUS_ERROR_LOCK;
  }

  if (inv->commandData.dischargeModeEnabled && enabled) {
    xSemaphoreGive(inv->commandData.mutex);
    return CINVERTER_STATUS_ERROR_DISCHARGE;
  }

  bool canSucc = true;

  if (!inv->commandData.inverterEnabled && enabled) {
    // TODO probably want to source this from information received form the inverter data instead
    // Transition from disabled to enabled
    // Inverter needs "inverter enable lockout" flag set, so send a disable message first
    canSucc |= SendCANCommand(
      inv->canInst,
      0.0f,
      VEHICLESTATE_INVERTER_FORWARD,
      false, // inverter disable
      false // discharge disable
    );
  }

  inv->commandData.inverterEnabled = enabled;

  // send inverter enable message
  canSucc |= SendCANCommand(
    inv->canInst,
    0.0f,
    VEHICLESTATE_INVERTER_FORWARD,
    enabled, // inverter enable/disable
    false // discharge disable
  );

  xSemaphoreGive(inv->commandData.mutex);
  return canSucc ? CINVERTER_STATUS_OK : CINVERTER_STATUS_ERROR_CAN;
}

CInverter_Status_T CInverter_SendInverterDischarge(CInverter_T* inv, bool dischargeModeEnabled)
{
  BaseType_t result = xSemaphoreTake(inv->commandData.mutex, portMAX_DELAY);
  if (pdTRUE != result) {
    return CINVERTER_STATUS_ERROR_LOCK;
  }

  if (inv->commandData.inverterEnabled && dischargeModeEnabled) {
    // Don't allow discharge mode with inverter still enabled
    xSemaphoreGive(inv->commandData.mutex);
    return CINVERTER_STATUS_ERROR_NOT_DISIABLED;
  }

  inv->commandData.dischargeModeEnabled = dischargeModeEnabled;
  if (dischargeModeEnabled) {
    // Can't enable torque output and discharge at the same time
    inv->commandData.inverterEnabled = false;
  }

  // send discharge message
  bool canSucc = SendCANCommand(
    inv->canInst,
    0.0f,
    VEHICLESTATE_INVERTER_FORWARD,
    false, // inverter disable
    dischargeModeEnabled // discharge enable
  );

  xSemaphoreGive(inv->commandData.mutex);
  return canSucc ? CINVERTER_STATUS_OK : CINVERTER_STATUS_ERROR_CAN;
}
