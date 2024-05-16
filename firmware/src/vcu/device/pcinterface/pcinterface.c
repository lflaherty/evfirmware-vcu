/*
 * pcinterface.c
 *
 *  Created on: Aug 2 2022
 *      Author: Liam Flaherty
 */
#include "pcinterface.h"

#include <stdio.h> // for snprintf
#include <string.h>
#include "tasktimer/tasktimer.h"
#include "uart/uart.h"
#include "can/can.h"
#include "gpio/gpio.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime2 = 100 / portTICK_PERIOD_MS; // 100ms // TODO fix testing infra so this isn't needed

// ------------------- Private methods -------------------
// ************ Request handlers ************
extern void PCInterface_HandleRequests(PCInterface_T* pcinterface);
extern void PCInterface_HandlePeriodic(PCInterface_T* pcinterface);

/**
 * @brief Sends all queued log messages to serial
 *
 * @param pcinterface PC Debug struct
 */
static void flushLogMessage(PCInterface_T* pcinterface)
{
  // Send saved log data to UART
  while (!xStreamBufferIsEmpty(pcinterface->logStreamHandle)) {
    // Construct message:
    memset(pcinterface->mfLogDataBuffer, 0U, PCINTERFACE_MSG_LOG_MSGLEN);
    uint8_t* msgData = MsgFrameEncode_InitFrame(&pcinterface->mfLogData);
    xStreamBufferReceive(pcinterface->logStreamHandle, msgData, PCINTERFACE_MSG_LOG_DATALEN, mBlockTime2);
    MsgFrameEncode_UpdateCRC(&pcinterface->mfLogData);

    // Duplicate the data on both ports (hardware probing is easier this way)
    UART_SendMessage(pcinterface->uartA, pcinterface->mfLogDataBuffer, PCINTERFACE_MSG_LOG_MSGLEN);
    UART_SendMessage(pcinterface->uartB, pcinterface->mfLogDataBuffer, PCINTERFACE_MSG_LOG_MSGLEN);
  }
}

static void periodicCanDebug(PCInterface_T* pcinterface)
{
  if (pcinterface->canDebugEnable) {
    uint32_t msgId = 0xAF;
    uint8_t data[8] = {0};
    uint32_t dlc = 8;

    // populate data with counter
    data[7] = 0x69;
    data[3] = (pcinterface->counter >> 24U) & 0xFF;
    data[2] = (pcinterface->counter >> 16U) & 0xFF;
    data[1] = (pcinterface->counter >>  8U) & 0xFF;
    data[0] = (pcinterface->counter >>  0U) & 0xFF;

    CAN_Status_T status = CAN_SendMessage(CAN_DEV1, msgId, data, dlc);
    if (status != CAN_STATUS_OK) {
      if (pcinterface->canErrorCounter % 10 == 0) {
        Log_Print(mLog, "PCInterface: CAN Error\n");
      }
      pcinterface->canErrorCounter++;
    }
  }
}

static void PCInterface_TaskMethod(PCInterface_T* pcinterface)
{
  // Wait for notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime2);
  if (notifiedValue > 0) {
    PCInterface_HandleRequests(pcinterface);
    PCInterface_HandlePeriodic(pcinterface);

    // Misc debug extras
    GPIO_TogglePin(pcinterface->pinToggle);
    periodicCanDebug(pcinterface);
    flushLogMessage(pcinterface);

    pcinterface->counter++;
  }
}

// LCOV_EXCL_START
static void PCInterface_Task(void* pvParameters)
{
  PCInterface_T* obj = (PCInterface_T*)pvParameters;

  while (1) {
    PCInterface_TaskMethod(obj);
  }
}
// LCOV_EXCL_STOP

// ------------------- Public methods -------------------
PCInterface_Status_T PCInterface_Init(
    Logging_T* logger,
    PCInterface_T* pcinterface)
{
  mLog = logger;
  pcinterface->log = logger; // TODO migrate to this way
  Log_Print(mLog, "PCInterface_Init begin\n");
  DEPEND_ON(logger, PCINTERFACE_STATUS_ERROR_DEPENDS);
  DEPEND_ON_STATIC(UART, PCINTERFACE_STATUS_ERROR_DEPENDS);
  DEPEND_ON_STATIC(TASKTIMER, PCINTERFACE_STATUS_ERROR_DEPENDS);

  pcinterface->counter = 0U;
  pcinterface->canErrorCounter = 0U;
  pcinterface->canDebugEnable = false;
  pcinterface->stateEnabled = false;
  pcinterface->controlEnabled = false;

  // Set up message frame encoders
  // Encoder init method is called when frame is used
  // But set common fields here once
  pcinterface->mfStateUpdate.crc = pcinterface->crc;
  pcinterface->mfStateUpdate.address = PCINTERFACE_MSG_DESTADDR_PC;
  pcinterface->mfStateUpdate.function = PCINTERFACE_MSG_STATEUPDATE_FUNCITION;
  pcinterface->mfStateUpdate.dataLen = PCINTERFACE_MSG_STATEUPDATE_DATALEN;
  pcinterface->mfStateUpdate.msgLen = PCINTERFACE_MSG_STATEUPDATE_MSGLEN;
  pcinterface->mfStateUpdate.buffer = pcinterface->mfStateUpdateBuffer;

  pcinterface->mfLogData.crc = pcinterface->crc;
  pcinterface->mfLogData.address = PCINTERFACE_MSG_DESTADDR_PC;
  pcinterface->mfLogData.function = PCINTERFACE_MSG_LOG_FUNCTION;
  pcinterface->mfLogData.dataLen = PCINTERFACE_MSG_LOG_DATALEN;
  pcinterface->mfLogData.msgLen = PCINTERFACE_MSG_LOG_MSGLEN;
  pcinterface->mfLogData.buffer = pcinterface->mfLogDataBuffer;

  pcinterface->mfDebugEncode.crc = pcinterface->crc;
  pcinterface->mfDebugEncode.address = PCINTERFACE_MSG_DESTADDR_PC;
  pcinterface->mfDebugEncode.function = PCINTERFACE_MSG_DEBUGTERM_FUNCTION;
  pcinterface->mfDebugEncode.dataLen = 0; // variable length, just init to 0
  pcinterface->mfDebugEncode.msgLen = 0;
  pcinterface->mfDebugEncode.buffer = pcinterface->mfDebugEncodeBuffer;

  // Setup message frame decoder
  pcinterface->mfDecode.crc = pcinterface->crc;
  pcinterface->mfDecode.msgLen = PCINTERFACE_MSG_COMMON_MSGLEN;
  if (!MsgFrameDecode_Init(&pcinterface->mfDecode)) {
    return PCINTERFACE_STATUS_ERROR_INIT;
  }

  // init debug term
  pcinterface->debugterm.next = 0U;

  // Create mutex lock & queue
  pcinterface->mutex = xSemaphoreCreateMutexStatic(&pcinterface->mutexBuffer);
  pcinterface->logStreamHandle = xStreamBufferCreateStatic(
      PCINTERFACE_LOG_STREAM_SIZE_BYTES,
      PCINTERFACE_LOG_STREAM_TRIGGER_LEVEL_BYTES,
      pcinterface->logStreamStorage,
      &pcinterface->logStreamStruct);

  // create main task
  pcinterface->taskHandle = xTaskCreateStatic(
      PCInterface_Task,
      "PCInterface",
      PCINTERFACE_STACK_SIZE,
      (void*)pcinterface,  /* Parameter passed as pointer */
      PCINTERFACE_TASK_PRIORITY,
      pcinterface->taskStack,
      &pcinterface->taskBuffer);

  // Register the task for timer notifications every 10ms (100Hz)
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&pcinterface->taskHandle, TASKTIMER_FREQUENCY_100HZ);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return PCINTERFACE_STATUS_ERROR_INIT;
  }

  // Register to receive logging data
  Logging_Status_T logStatus =
      Log_SetSerialStream(mLog, pcinterface->logStreamHandle);
  if (LOGGING_STATUS_OK != logStatus) {
    return PCINTERFACE_STATUS_ERROR_INIT;
  }

  // Register to receive serial data
  pcinterface->recvStreamHandle = xStreamBufferCreateStatic(
      PCINTERFACE_RECV_STREAM_SIZE_BYTES,
      PCINTERFACE_RECV_STREAM_TRIGGER_LEVEL_BYTES,
      pcinterface->recvStreamStorage,
      &pcinterface->recvStreamStruct);
  if (UART_STATUS_OK != UART_SetRecvStream(pcinterface->uartA, pcinterface->recvStreamHandle)) {
    return PCINTERFACE_STATUS_ERROR_INIT;
  }
  if (UART_STATUS_OK != UART_SetRecvStream(pcinterface->uartB, pcinterface->recvStreamHandle)) {
    return PCINTERFACE_STATUS_ERROR_INIT;
  }

  REGISTER(pcinterface, PCINTERFACE_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "PCInterface_Init complete\n");
  return PCINTERFACE_STATUS_OK;
}

PCInterface_Status_T PCInterface_SetVehicleState(
    PCInterface_T* pcinterface,
    VehicleState_T* state)
{
  DEPEND_ON(state, PCINTERFACE_STATUS_ERROR_DEPENDS);

  pcinterface->state = state;
  pcinterface->stateEnabled = true;

  return PCINTERFACE_STATUS_OK;
}

PCInterface_Status_T PCInterface_SetVehicleControl(
    PCInterface_T* pcinterface,
    VehicleControl_T* control)
{
  DEPEND_ON(control, PCINTERFACE_STATUS_ERROR_DEPENDS);

  pcinterface->control = control;
  pcinterface->controlEnabled = true;

  return PCINTERFACE_STATUS_OK;
}
