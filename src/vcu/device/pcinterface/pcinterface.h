/*
 * pcinterface.h
 *
 * Driver for comm interface with PC via RS232. Used for receiving and
 * transmitting config data, logs, and debug data.
 *
 *  Created on: Aug 2 2022
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_PCINTERFACE_PCINTERFACE_H_
#define DEVICE_PCINTERFACE_PCINTERFACE_H_

#include "lib/logging/logging.h"

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "lib/depends/depends.h"
#include "lib/crc/crc.h"
#include "comm/uart/msgframeencode.h"
#include "vehicleInterface/vehicleState/vehicleState.h"

typedef enum
{
  PCINTERFACE_STATUS_OK             = 0x00,
  PCINTERFACE_STATUS_ERROR_INIT     = 0x01,
  PCINTERFACE_STATUS_ERROR_DEPENDS  = 0x02,
  PCINTERFACE_STATUS_ERROR_SIZE     = 0x03,
} PCInterface_Status_T;

#define PCINTERFACE_STACK_SIZE 2000U
#define PCINTERFACE_TASK_PRIORITY 3U
#define PCINTERFACE_LOG_STREAM_SIZE_BYTES 2048U
#define PCINTERFACE_LOG_STREAM_TRIGGER_LEVEL_BYTES 1U
#define PCINTERFACE_RECV_STREAM_SIZE_BYTES 2048U
#define PCINTERFACE_RECV_STREAM_TRIGGER_LEVEL_BYTES 1U

#define PCINTERFACE_MSG_DESTADDR 0x02

#define PCINTERFACE_MSG_STATEUPDATE_FUNCITION 0x01
#define PCINTERFACE_MSG_STATEUPDATE_DATALEN   7U
#define PCINTERFACE_MSG_STATEUPDATE_MSGLEN    18U
#define PCINTERFACE_MSG_LOG_FUNCTION 0x02
#define PCINTERFACE_MSG_LOG_DATALEN  32U
#define PCINTERFACE_MSG_LOG_MSGLEN   43U

typedef struct
{
  UART_HandleTypeDef* huartA;
  UART_HandleTypeDef* huartB;
  CRC_T* crc;
  VehicleState_T* state; // Vehicle state object to fetch data from

  // ******* Internal use *******
  bool canDebugEnable; // TODO remove
  uint32_t counter;
  uint32_t canErrorCounter;

  // Mutex lock
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;

  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[PCINTERFACE_STACK_SIZE];

  // Stream buffer objects for receiving logs
  uint8_t logStreamStorage[PCINTERFACE_LOG_STREAM_SIZE_BYTES];
  StaticStreamBuffer_t logStreamStruct;
  StreamBufferHandle_t logStreamHandle;

  // Stream buffer objects for receiving uart bytes
  uint8_t recvStreamStorage[PCINTERFACE_RECV_STREAM_SIZE_BYTES];
  StaticStreamBuffer_t recvStreamStruct;
  StreamBufferHandle_t recvStreamHandle;

  // Message frames for encoding
  MsgFrameEncode_T mfStateUpdate;
  uint8_t mfStateUpdateBuffer[PCINTERFACE_MSG_STATEUPDATE_MSGLEN];
  MsgFrameEncode_T mfLogData;
  uint8_t mfLogDataBuffer[PCINTERFACE_MSG_LOG_MSGLEN];

  REGISTERED_MODULE();
} PCInterface_T;

/**
 * @brief Initialize the PC Debug interface.
 * 
 * @param logger Pointer to system logger.
 * @param pcinterface PCInterface struct
 * @return PCINTERFACE_STATUS_OK if init successful 
 */
PCInterface_Status_T PCInterface_Init(
    Logging_T* logger,
    PCInterface_T* pcinterface);

#endif // DEVICE_PCINTERFACE_PCINTERFACE_H_
