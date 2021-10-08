/*
 * vehicleState.h
 *
 *  Created on: Oct 8 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATE_H_
#define VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATE_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "lib/logging/logging.h"

#include "vehicleStateTypes.h"

typedef enum
{
  VEHICLESTATE_STATUS_OK          = 0x00U,
  VEHCILESTATE_STATUS_ERROR_INIT  = 0x01U,
  VEHCILESTATE_STATUS_ERROR_QUEUE = 0x02U
} VehicleState_Status_T;

typedef enum
{
  VEHICLESTATE_TYPE_FLOAT
} VehicleState_QueuedDataType_T;

/**
 * Used to queue multiple types of data
 * Internal use only.
 */
typedef struct
{
  union
  {
    float dFloat;
  } data;
  void* dest;
  VehicleState_QueuedDataType_T dataType;
} VehicleState_QueuedData_T;


#define VEHCILESTATE_STACK_SIZE 2000
#define VEHICLESTATE_TASK_PRIORITY 3
#define VEHICLESTATE_QUEUE_LENGTH 128
#define VEHICLESTATE_QUEUE_DATA_SIZE sizeof(VehicleState_QueuedData_T)

typedef struct
{
  // ******* Shared *******
  // Vehicle data
  VehicleState_Data_T data;

  // Mutex lock - must lock this before accessing data
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;

  // ******* Internal use *******
  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[VEHCILESTATE_STACK_SIZE];

  // Queue
  QueueHandle_t dataQueueHandle;
  StaticQueue_t dataQueueBuffer;
  uint8_t dataQueueStorageArea[VEHICLESTATE_QUEUE_LENGTH*VEHICLESTATE_QUEUE_DATA_SIZE];
} VehicleState_T;

/**
 * @brief Initialize the vehicle state process
 * @param logger Pointer to system logger
 * @param state Pointer to VehicleState object
 * @returns Success status. VEHCILESTATE_STATUS_OK if successful.
 */
VehicleState_Status_T VehicleState_Init(Logging_T* logger, VehicleState_T* state);

/**
 * @brief Schedule pushing a float data field into the state
 * @param state Pointer to VehicleState object
 * @param dest Pointer to destination (contained within state->data)
 * @returns Success status. VEHCILESTATE_STATUS_OK if successful.
 */
VehicleState_Status_T VehicleState_PushFieldf(VehicleState_T* state, void* dest, float value);


#endif /* VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATE_H_ */
