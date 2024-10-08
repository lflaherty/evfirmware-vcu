/*
 * orionBms.c
 *
 *  Created on: 15 Apr 2023
 *      Author: Liam Flaherty
 */

#include "bms.h"

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f7xx_hal.h"

#include "logging/logging.h"
#include "can/can.h"
#include "tasktimer/tasktimer.h"

#include "orionBmsCAN.h"
#include "orionBmsDataConversions.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms

// ------------------- Private methods -------------------
static void HandleMsg_MaxCellState(const CAN_DataFrame_T* data, VehicleState_T* state)
{
  if (data->dlc != 8) {
    return;
  }
  const uint8_t* msg = data->data;

  int16_t maxCellTempMsg = 0;
  maxCellTempMsg |= msg[0];
  maxCellTempMsg |= (int16_t)(msg[1] << 8);

  int16_t maxCellVoltageMsg = 0;
  maxCellVoltageMsg |= (int16_t)msg[3];
  maxCellVoltageMsg |= (int16_t)(msg[4] << 8);

  uint8_t maxCellTempID = data->data[2];
  uint8_t maxCellVoltageID = data->data[5];

  float maxCellTemp = msgToTemperature(maxCellTempMsg);
  float maxCellVoltage = msgToVoltageLow(maxCellVoltageMsg);

  // send to vehicle state
  if (VehicleState_AccessAcquire(state)) {
    state->data.battery.maxCellTemperature = maxCellTemp;
    state->data.battery.maxCellTemperatureCellID = maxCellTempID;
    state->data.battery.maxCellVoltage = maxCellVoltage;
    state->data.battery.maxCellVoltageCellID = maxCellVoltageID;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_MinCellState(const CAN_DataFrame_T* data, VehicleState_T* state)
{
  if (data->dlc != 8) {
    return;
  }
  const uint8_t* msg = data->data;

  int16_t minCellTempMsg = 0;
  minCellTempMsg |= (int16_t)msg[0];
  minCellTempMsg |= (int16_t)(msg[1] << 8);

  int16_t minCellVoltageMsg = 0;
  minCellVoltageMsg |= (int16_t)msg[3];
  minCellVoltageMsg |= (int16_t)(msg[4] << 8);

  uint8_t minCellTempID = data->data[2];
  uint8_t minCellVoltageID = data->data[5];

  float minCellTemp = msgToTemperature(minCellTempMsg);
  float minCellVoltage = msgToVoltageLow(minCellVoltageMsg);

  // send to vehicle state
  if (VehicleState_AccessAcquire(state)) {
    state->data.battery.minCellTemperature = minCellTemp;
    state->data.battery.minCellTemperatureCellID = minCellTempID;
    state->data.battery.minCellVoltage = minCellVoltage;
    state->data.battery.minCellVoltageCellID = minCellVoltageID;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_PackState(const CAN_DataFrame_T* data, VehicleState_T* state)
{
  if (data->dlc != 8) {
    return;
  }
  const uint8_t* msg = data->data;

  int16_t dcCurrentMsg = 0;
  dcCurrentMsg |= (int16_t)msg[0];
  dcCurrentMsg |= (int16_t)(msg[1] << 8);

  int16_t dcVoltageMsg = 0;
  dcVoltageMsg |= (int16_t)msg[2];
  dcVoltageMsg |= (int16_t)(msg[3] << 8);

  uint16_t socMsg = 0;
  socMsg |= (uint16_t)msg[4];
  socMsg |= (uint16_t)(msg[5] << 8);

  float dcCurrent = msgToCurrent(dcCurrentMsg);
  float dcVoltage = msgToVoltageHigh(dcVoltageMsg);
  float soc = msgToStateOfChargePct(socMsg);

  // send to vehicle state
  if (VehicleState_AccessAcquire(state)) {
    state->data.battery.dcCurrent = dcCurrent;
    state->data.battery.dcVoltage = dcVoltage;
    state->data.battery.stateOfCarge = soc;
  }
  VehicleState_AccessRelease(state);
}

static void HandleMsg_Status(const CAN_DataFrame_T* data, VehicleState_T* state)
{
  if (data->dlc != 8) {
    return;
  }
  const uint8_t* msg = data->data;

  uint8_t counter = msg[0];
  uint8_t popualtedCells = msg[1];
  uint16_t failsafeStatus = 0;
  failsafeStatus |= (uint16_t)msg[2];
  failsafeStatus |= (uint16_t)(msg[3] << 8);

  // send to vehicle state
  if (VehicleState_AccessAcquire(state)) {
    state->data.battery.bmsPopulatedCells = popualtedCells;
    state->data.battery.bmsCounter = counter;
    state->data.battery.bmsFailsafeStatus = failsafeStatus;
  }
  VehicleState_AccessRelease(state);
}

static void BMSProcessing(BMS_T* bms)
{
  // Wait for 10ms notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    CAN_DataFrame_T queuedData;
    while (pdTRUE == xQueueReceive(bms->canDataQueueHandle, &queuedData, 0U)) {
      if (queuedData.busInstance != bms->canInst) {
        // Wrong bus, throw away
        continue;
      }

      switch (queuedData.msgId) {
        case BMS_CAN_ID_MAXCELLSTATE:
          HandleMsg_MaxCellState(&queuedData, bms->vehicleState);
          break;
        case BMS_CAN_ID_MINCELLSTATE:
          HandleMsg_MinCellState(&queuedData, bms->vehicleState);
          break;
        case BMS_CAN_ID_PACKSTATE:
          HandleMsg_PackState(&queuedData, bms->vehicleState);
          break;
        case BMS_CAN_ID_STATUS:
          HandleMsg_Status(&queuedData, bms->vehicleState);
          break;
        default:
          // Don't know this message ID - throw away message
          break;
      }
    }
  }

}

// LCOV_EXCL_START
static void BMSProcessing_Task(void* pvParameters)
{
  BMS_T* bms = (BMS_T*)pvParameters;

  while (1) {
    BMSProcessing(bms);
  }
}
// LCOV_EXCL_STOP

// ------------------- Public methods -------------------
BMS_Status_T BMS_Init(Logging_T* logger, BMS_T* bms)
{
  mLog = logger;
  Log_Print(mLog, "BMS_Init begin\n");
  DEPEND_ON(logger, BMS_STATUS_ERROR_DEPENDS);
  DEPEND_ON_STATIC(CAN, BMS_STATUS_ERROR_DEPENDS);

  // Create queue for receiving CAN data
  bms->canDataQueueHandle = xQueueCreateStatic(
      BMS_QUEUE_LENGTH,
      BMS_QUEUE_DATA_SIZE,
      bms->canDataQueueStorageArea,
      &bms->canDataQueueBuffer);

  // Create RTOS task
  bms->taskHandle = xTaskCreateStatic(
      BMSProcessing_Task,
      "OrionBMS",
      BMS_STACK_SIZE,   /* Stack size */
      (void*)bms,  /* Parameter passed as pointer */
      BMS_TASK_PRIORITY,
      bms->taskStack,
      &bms->taskBuffer);

  // Register RTOS task for 100Hz updates
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&bms->taskHandle, TASKTIMER_FREQUENCY_100HZ);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return BMS_STATUS_ERROR_INIT;
  }

  // Start receiving CAN data
  CAN_Status_T callbackRegStatus = CAN_RegisterQueue(
      bms->canInst,
      BMS_CAN_DEVICEID,
      BMS_CAN_DEVICEIDMASK,
      bms->canDataQueueHandle);
  if (CAN_STATUS_OK != callbackRegStatus) {
    return BMS_STATUS_ERROR_CAN;
  }

  REGISTER(bms, BMS_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "BMS_Init complete\n");
  return BMS_STATUS_OK;
}
