/*
 * pcdebug.h
 *
 *  Created on: Aug 2 2022
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_PCDEBUG_PCDEBUG_H_
#define DEVICE_PCDEBUG_PCDEBUG_H_

#include "lib/logging/logging.h"

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "lib/crc/crc.h"
#include "comm/uart/msgframeencode.h"

typedef enum
{
  PCDEBUG_STATUS_OK         = 0x00,
  PCDEBUG_STATUS_ERROR_INIT = 0x01
} PCDebug_Status_T;

#define PCDEBUG_STACK_SIZE 2000U
#define PCDEBUG_TASK_PRIORITY 3U
#define PCDEBUG_LOG_STREAM_SIZE_BYTES 2048U
#define PCDEBUG_LOG_STREAM_TRIGGER_LEVEL_BYTES 1U
#define PCDEBUG_RECV_STREAM_SIZE_BYTES 2048U
#define PCDEBUG_RECV_STREAM_TRIGGER_LEVEL_BYTES 1U

#define PCDEBUG_MSG_DESTADDR 0x02

#define PCDEBUG_MSG_LOG_FUNCTION 0x02
#define PCDEBUG_MSG_LOG_DATALEN  32U
#define PCDEBUG_MSG_LOG_MSGLEN   43U

typedef struct
{
  UART_HandleTypeDef* huartA;
  UART_HandleTypeDef* huartB;
  CRC_T* crc;

  // ******* Internal use *******
  uint32_t counter;
  uint32_t canErrorCounter;

  // Mutex lock
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;

  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[PCDEBUG_STACK_SIZE];

  // Stream buffer objects for receiving logs
  uint8_t logStreamStorage[PCDEBUG_LOG_STREAM_SIZE_BYTES];
  StaticStreamBuffer_t logStreamStruct;
  StreamBufferHandle_t logStreamHandle;

  // Stream buffer objects for receiving uart bytes
  uint8_t recvStreamStorage[PCDEBUG_RECV_STREAM_SIZE_BYTES];
  StaticStreamBuffer_t recvStreamStruct;
  StreamBufferHandle_t recvStreamHandle;

  // Message frames for encoding
  MsgFrameEncode_T mfLogData;
  uint8_t mfLogDataBuffer[PCDEBUG_MSG_LOG_MSGLEN];
} PCDebug_T;

/**
 * @brief Initialize the PC Debug interface.
 * 
 * @param logger Pointer to system logger.
 * @param pcdebug PCDebug struct
 * @return PCDEBUG_STATUS_OK if init successful 
 */
PCDebug_Status_T PCDebug_Init(
    Logging_T* logger,
    PCDebug_T* pcdebug);

#endif // DEVICE_PCDEBUG_PCDEBUG_H_