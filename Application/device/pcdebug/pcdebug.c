/*
 * pcdebug.c
 *
 *  Created on: Aug 2 2022
 *      Author: Liam Flaherty
 */
#include "pcdebug.h"

#include <stdio.h> // for snprintf
#include <string.h>
#include "time/tasktimer/tasktimer.h"
#include "comm/uart/uart.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms
static const uint32_t tickRateMs = 100U; // 100 ms

// ------------------- Private methods -------------------
/**
 * @brief Sends all queued log messages to serial
 *
 * @param pcdebug PC Debug struct
 */
static void flushLogMessage(PCDebug_T* pcdebug)
{
  while (!xStreamBufferIsEmpty(pcdebug->logStreamHandle)) {
    // Construct message:
    memset(pcdebug->mfLogDataBuffer, 0U, PCDEBUG_MSG_LOG_MSGLEN);
    uint8_t* msgData = MsgFrameEncode_InitFrame(&pcdebug->mfLogData);
    xStreamBufferReceive(pcdebug->logStreamHandle, msgData, PCDEBUG_MSG_LOG_DATALEN, mBlockTime);
    MsgFrameEncode_UpdateCRC(&pcdebug->mfLogData);

    UART_SendMessage(pcdebug->huart, pcdebug->mfLogDataBuffer, PCDEBUG_MSG_LOG_MSGLEN);
  }
}

static void PCDebug_TaskMethod(PCDebug_T* pcdebug)
{
  // Wait for notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    if (pdFALSE == xStreamBufferIsEmpty(pcdebug->recvStreamHandle)) {
      // For now just print them to log...
      Log_Print(mLog, "Recevied serial bytes: ");
      while (!xStreamBufferIsEmpty(pcdebug->recvStreamHandle)) {
        uint8_t recvByte = 0;
        xStreamBufferReceive(pcdebug->recvStreamHandle, &recvByte, 1U, mBlockTime);

        char logBuffer[LOGGING_MAX_MSG_LEN] = { 0 };
        snprintf(logBuffer, LOGGING_MAX_MSG_LEN, "0x%02x ", recvByte);
        Log_Print(mLog, logBuffer);
      }
      Log_Print(mLog, "\n");
    }

    flushLogMessage(pcdebug);
  }
}

static void PCDebug_Task(void* pvParameters)
{
  PCDebug_T* obj = (PCDebug_T*)pvParameters;

  while (1) {
    PCDebug_TaskMethod(obj);
  }
}

// ------------------- Public methods -------------------
PCDebug_Status_T PCDebug_Init(
    Logging_T* logger,
    PCDebug_T* pcdebug)
{
  mLog = logger;
  Log_Print(mLog, "PCDebug_Init begin\n");

  // Set up message frames
  pcdebug->mfLogData.hcrc = pcdebug->hcrc;
  pcdebug->mfLogData.address = PCDEBUG_MSG_DESTADDR;
  pcdebug->mfLogData.function = PCDEBUG_MSG_LOG_FUNCTION;
  pcdebug->mfLogData.dataLen = PCDEBUG_MSG_LOG_DATALEN;
  pcdebug->mfLogData.msgLen = PCDEBUG_MSG_LOG_MSGLEN;
  pcdebug->mfLogData.buffer = pcdebug->mfLogDataBuffer;

  // Create mutex lock
  pcdebug->mutex = xSemaphoreCreateMutexStatic(&pcdebug->mutexBuffer);

  // create main task
  pcdebug->taskHandle = xTaskCreateStatic(
      PCDebug_Task,
      "PCDebug",
      PCDEBUG_STACK_SIZE,
      (void*)pcdebug,  /* Parameter passed as pointer */
      PCDEBUG_TASK_PRIORITY,
      pcdebug->taskStack,
      &pcdebug->taskBuffer);

  // Register the task for timer notifications every 10ms (100Hz)
  uint16_t timerDivider = tickRateMs * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&pcdebug->taskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return PCDEBUG_STATUS_ERROR_INIT;
  }

  // Register to receive logging data
  pcdebug->logStreamHandle = xStreamBufferCreateStatic(
      PCDEBUG_LOG_STREAM_SIZE_BYTES,
      PCDEBUG_LOG_STREAM_TRIGGER_LEVEL_BYTES,
      pcdebug->logStreamStorage,
      &pcdebug->logStreamStruct);
  Logging_Status_T logStatus =
      Log_SetSerialStream(mLog, pcdebug->logStreamHandle);
  if (LOGGING_STATUS_OK != logStatus) {
    return PCDEBUG_STATUS_ERROR_INIT;
  }

  // Register to receive serial data
  pcdebug->recvStreamHandle = xStreamBufferCreateStatic(
      PCDEBUG_RECV_STREAM_SIZE_BYTES,
      PCDEBUG_RECV_STREAM_TRIGGER_LEVEL_BYTES,
      pcdebug->recvStreamStorage,
      &pcdebug->recvStreamStruct);
  UART_Status_T uartStatus =
      UART_SetRecvStream(pcdebug->huart, pcdebug->recvStreamHandle);
  if (UART_STATUS_OK != uartStatus) {
    return PCDEBUG_STATUS_ERROR_INIT;
  }

  Log_Print(mLog, "PCDebug_Init complete\n");
  return PCDEBUG_STATUS_OK;
}
