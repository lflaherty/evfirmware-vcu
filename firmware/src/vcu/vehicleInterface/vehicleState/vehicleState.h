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

#include "depends/depends.h"
#include "logging/logging.h"

#include "vehicleStateTypes.h"

typedef enum
{
  VEHICLESTATE_STATUS_OK            = 0x00U,
  VEHICLESTATE_STATUS_ERROR_INIT    = 0x01U,
  VEHICLESTATE_STATUS_ERROR_QUEUE   = 0x02U,
  VEHICLESTATE_STATUS_ERROR_SIZE    = 0x03U,
  VEHICLESTATE_STATUS_ERROR_DEPENDS = 0x04U,
} VehicleState_Status_T;

#define VEHICLESTATE_QUEUE_MAX_VALUE_SIZE 4 /* bytes */

/**
 * Used to queue multiple types of data
 * Internal use only.
 */
typedef struct
{
  uint8_t data[VEHICLESTATE_QUEUE_MAX_VALUE_SIZE];
  size_t dataSize;
  void* dest;
} VehicleState_QueuedData_T;


#define VEHICLESTATE_STACK_SIZE 2000
#define VEHICLESTATE_TASK_PRIORITY 3
#define VEHICLESTATE_QUEUE_LENGTH 256
#define VEHICLESTATE_QUEUE_DATA_SIZE sizeof(VehicleState_QueuedData_T)

typedef struct
{
  // ******* Shared *******
  // Vehicle data
  VehicleState_Data_T data;

  // Mutex lock - must lock this via VehicleState_AccessAcquire before
  // accessing data
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;

  // ******* Internal use *******
  REGISTERED_MODULE();
} VehicleState_T;

/**
 * @brief Initialize the vehicle state process
 * @param logger Pointer to system logger
 * @param state Pointer to VehicleState object
 * @returns Success status. VEHICLESTATE_STATUS_OK if successful.
 */
VehicleState_Status_T VehicleState_Init(Logging_T* logger, VehicleState_T* state);

/**
 * @brief Thread safe copy from state->data to dest
 * 
 * @param state Source of data
 * @param dest Location to copy to
 * @return true Copy was successful
 * @return false Copy failed (thread fail)
 */
bool VehicleState_CopyState(VehicleState_T* state, VehicleState_Data_T* dest);

/**
 * @brief Lock the mutex for access.
 * Only use this to batch read a number of variables. Do not leave locked.
 * 
 * @param state Pointer to VehicleState struct
 * @return true If mutex was granted
 * @return false If mutex failed 
 */
bool VehicleState_AccessAcquire(VehicleState_T* state);

/**
 * @brief Corresponding unlock for VehicleState_AccessAcquire.
 * Only use this to batch read a number of variables. Do not leave locked.
 * 
 * @param state Pointer to VehicleState struct
 * @return true If mutex was released
 * @return false If failed
 */
bool VehicleState_AccessRelease(VehicleState_T* state);


#endif /* VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATE_H_ */
