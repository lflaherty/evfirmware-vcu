/*
 * periodicupdates.c
 *
 * Implements logic for periodic messages
 *
 *  Created on: Dec 2 2022
 *      Author: Liam Flaherty
 */

#include "pcinterface.h"
#include <stdio.h> /* for snprintf */
#include <string.h>

#include "fieldId.h"

#include "uart/uart.h"

#define COUNT_1HZ (uint32_t)100U

/**
 * @brief Transmits a state field broadcast message
 * 
 * @param pcinterface 
 * @param fieldId 
 * @param fieldSize 
 * @param field 
 */
static void sendStateField(
    PCInterface_T* pcinterface,
    const uint16_t fieldId,
    const size_t fieldSize,
    const uint32_t field)
{
  if (fieldSize > 4) {
    // state update message only supports 4 byte fields
    return;
  }

  // Update message buffer contents
  uint8_t* payload = MsgFrameEncode_InitFrame(&pcinterface->mfStateUpdate);
  payload[0] = (uint8_t)((fieldId >> 8) & 0xFF);
  payload[1] = (uint8_t)(fieldId & 0xFF);
  payload[2] = (uint8_t)(fieldSize & 0xFF);
  payload[3] = (uint8_t)((field >> 24) & 0xFF);
  payload[4] = (uint8_t)((field >> 16) & 0xFF);
  payload[5] = (uint8_t)((field >> 8) & 0xFF);
  payload[6] = (uint8_t)(field & 0xFF);

  MsgFrameEncode_UpdateCRC(&pcinterface->mfStateUpdate);

  // Send to both interfaces
  UART_SendMessage(
      pcinterface->uartA,
      pcinterface->mfStateUpdateBuffer,
      PCINTERFACE_MSG_STATEUPDATE_MSGLEN);
  UART_SendMessage(
      pcinterface->uartB,
      pcinterface->mfStateUpdateBuffer,
      PCINTERFACE_MSG_STATEUPDATE_MSGLEN);
}

static void sendStateFieldf(
    PCInterface_T* pcinterface,
    const uint16_t fieldId,
    const float field)
{
  _Static_assert(sizeof(float) == sizeof(uint32_t), "impl assumes float is 32bits");

  uint32_t fieldU32;
  memcpy(&fieldU32, &field, sizeof(uint32_t));

  sendStateField(pcinterface, fieldId, sizeof(uint32_t), fieldU32);
}

/**
 * @brief Send SDC state data message
 * Not thread safe. Data must be a safe copy of state.
 * 
 * @param pcinterface PCInterface object
 * @param data Pointer to copy of state data.
 */
static void sendStateSdc(
    PCInterface_T* pcinterface,
    VehicleState_SDC_T* data)
{
  uint8_t tmpSdc = 0;
  tmpSdc |= (uint8_t)((data->bms & 0x1) << 0);
  tmpSdc |= (uint8_t)((data->bspd & 0x1) << 1);
  tmpSdc |= (uint8_t)((data->imd & 0x1) << 2);
  tmpSdc |= (uint8_t)((data->out & 0x1) << 3);
  sendStateField(
      pcinterface,
      PCCONTROLLER_FIELDID_SDC,
      sizeof(tmpSdc), tmpSdc);
}

/**
 * @brief Send PDM state data message
 * Not thread safe. Data must be a safe copy of state.
 * 
 * @param pcinterface PCInterface object
 * @param data Pointer to copy of state data.
 */
static void sendStatePdm(
    PCInterface_T* pcinterface,
    VehicleState_GLV_T* data)
{
  _Static_assert(VEHICLESTATE_MAXPDM_CHANNELS <= 8, "Can't fit >8 channels in uint8_t");

  uint8_t tmpPdm = 0;
  for (uint8_t i = 0; i < 6; ++i) {
    tmpPdm |= (uint8_t)((data->pdmChState[i] & 0x1) << i);
  }

  sendStateField(
      pcinterface,
      PCCONTROLLER_FIELDID_PDM,
      sizeof(tmpPdm), tmpPdm);
}

/**
 * @brief Send battery state data messages.
 * Not thread safe. Data must be a safe copy of state.
 * 
 * @param pcinterface PCInterface object
 * @param data Pointer to copy of state data.
 */
static void sendStateBattery(
    PCInterface_T* pcinterface,
    VehicleState_Battery_T* data)
{
  _Static_assert(sizeof(float) <= 4, "float size");

  sendStateFieldf(pcinterface,
      PCCONTROLLER_FIELDID_BATTERY_MAXCELLVOLT,
      data->maxCellVoltage);
  sendStateField(pcinterface,
      PCCONTROLLER_FIELDID_BATTERY_MAXCELLVOLTID,
      sizeof(data->maxCellVoltageCellID), data->maxCellVoltageCellID);
  sendStateFieldf(pcinterface,
      PCCONTROLLER_FIELDID_BATTERY_MAXCELLTEMP,
      data->maxCellTemperature);
  sendStateField(pcinterface,
      PCCONTROLLER_FIELDID_BATTERY_MAXCELLTEMPID,
      sizeof(data->minCellTemperatureCellID), data->minCellTemperatureCellID);
  sendStateFieldf(pcinterface,
      PCCONTROLLER_FIELDID_BATTERY_DCCURRENT,
      data->dcCurrent);
  sendStateFieldf(pcinterface,
      PCCONTROLLER_FIELDID_BATTERY_DCVOLTAGE,
      data->dcVoltage);
  sendStateFieldf(pcinterface,
      PCCONTROLLER_FIELDID_BATTERY_SOC,
      data->stateOfCarge);
  sendStateField(pcinterface,
      PCCONTROLLER_FIELDID_BATTERY_COUNTER,
      sizeof(data->bmsCounter), data->bmsCounter);
  sendStateField(pcinterface,
      PCCONTROLLER_FIELDID_BMS_FAULTINDICATOR,
      sizeof(data->bmsFaultIndicator), data->bmsFaultIndicator);
}

/**
 * @brief At 1Hz, transmit internal state
 * 
 * @param pcinterface 
 */
static void periodicStateUpdate(PCInterface_T* pcinterface)
{
  if (!pcinterface->stateEnabled) {
    // PCInterface_SetVehicleState hasn't been called yet
    return;
  }

  // Periodic task runs at 100Hz, but only want to tx the state at 1Hz
  if (pcinterface->counter % COUNT_1HZ != 0U) {
    return;
  }

  // Create a copy of the internal state
  // (Don't hold a lock on the vehicle state just to transmit this data)
  VehicleState_Data_T data;
  if (!VehicleState_CopyState(pcinterface->state, &data)) {
    return;
  }

  sendStateSdc(pcinterface, &data.vehicle.sdc);
  sendStatePdm(pcinterface, &data.glv);
  sendStateBattery(pcinterface, &data.battery);
}

/**
 * @brief Sends periodic message updates
 * 
 * @param pcinterface 
 */
void PCInterface_HandlePeriodic(PCInterface_T* pcinterface)
{
  periodicStateUpdate(pcinterface);
}
