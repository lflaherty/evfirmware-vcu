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
#include "io/gpio/gpio.h"
#include "comm/uart/uart.h"
#include "comm/uart/msgframeencode.h"
#include "comm/uart/msgframedecode.h"
#include "vehicleInterface/vehicleState/vehicleState.h"
#include "vehicleInterface/vehicleControl/vehicleControl.h"

#include "messages.h"

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

typedef struct
{
  UART_Device_T uartA;
  UART_Device_T uartB;
  CRC_T* crc;
  GPIO_T* pinToggle; // toggled at process refresh rate

  // ******* Internal use *******
  bool canDebugEnable; // TODO remove
  uint32_t counter;
  uint32_t canErrorCounter;

  Logging_T* log; // TODO move towards this approach

  VehicleState_T* state; // Vehicle state object to fetch data from
  VehicleControl_T* control;
  bool stateEnabled; // whether state pointer is set and ready
  bool controlEnabled; // whether control pointer is set and ready

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

  // Message frame for decoding
  MsgFrameDecode_T mfDecode;

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

/**
 * @brief Sets the internal vehicle state pointer and enables
 * related functionality such as periodic state data outputs.
 * 
 * @param pcinterface PCInterface struct
 * @param state VehicleState pointer
 * @return PCINTERFACE_STATUS_OK if successful
 */
PCInterface_Status_T PCInterface_SetVehicleState(
    PCInterface_T* pcinterface,
    VehicleState_T* state);

/**
 * @brief Sets the internal vehicle control pointer and enables
 * related functionality such as test messages.
 * 
 * @param pcinterface PCInterface struct
 * @param control VehicleControl pointer
 * @return PCINTERFACE_STATUS_OK if successful
 */
PCInterface_Status_T PCInterface_SetVehicleControl(
    PCInterface_T* pcinterface,
    VehicleControl_T* control);

#endif // DEVICE_PCINTERFACE_PCINTERFACE_H_
