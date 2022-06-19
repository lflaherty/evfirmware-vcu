/*
 * vehicleState.c
 *
 *  Created on: Oct 8 2021
 *      Author: Liam Flaherty
 */

#include "vehicleState.h"

#include <string.h>

#include "time/tasktimer/tasktimer.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms

// ------------------- Private methods -------------------
static void StateProcessing(VehicleState_T* vehicleState)
{
  // Wait for notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {

    // Acquire lock on data
    if (pdTRUE == xSemaphoreTake(vehicleState->mutex, portMAX_DELAY)) {
      // Process any waiting data
      VehicleState_QueuedData_T queuedData;
      while (pdTRUE == xQueueReceive(vehicleState->dataQueueHandle, &queuedData, 10U)) {

        // copy data out
        if (queuedData.dest != NULL) {
          memcpy(queuedData.dest, queuedData.data, queuedData.dataSize);
        }

      }

      xSemaphoreGive(vehicleState->mutex);
    }

  }
}

static void StateProcessing_Task(void* pvParameters)
{
  VehicleState_T* obj = (VehicleState_T*)pvParameters;

  while (1) {
    StateProcessing(obj);
  }
}

// ------------------- Public methods -------------------
VehicleState_Status_T VehicleState_Init(Logging_T* logger, VehicleState_T* state)
{
  mLog = logger;
  logPrintS(mLog, "VehicleState_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  memset(&state->data, 0, sizeof(state->data)); // initialize data to 0

  // Create queue
  state->dataQueueHandle = xQueueCreateStatic(
      VEHICLESTATE_QUEUE_LENGTH,
      VEHICLESTATE_QUEUE_DATA_SIZE,
      state->dataQueueStorageArea,
      &state->dataQueueBuffer);

  // Create mutex lock
  state->mutex = xSemaphoreCreateBinaryStatic(&state->mutexBuffer);

  // create main task
  state->taskHandle = xTaskCreateStatic(
      StateProcessing_Task,
      "VehicleState",
      VEHICLESTATE_STACK_SIZE,   /* Stack size */
      (void*)state,  /* Parameter passed as pointer */
      VEHICLESTATE_TASK_PRIORITY,
      state->taskStack,
      &state->taskBuffer);

  // Register the task for timer notifications every 10ms (100Hz)
  uint16_t timerDivider = 10 * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&state->taskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return VEHCILESTATE_STATUS_ERROR_INIT;
  }

  logPrintS(mLog, "VehicleState_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return VEHICLESTATE_STATUS_OK;
}

//------------------------------------------------------------------------------
VehicleState_Status_T VehicleState_PushField(VehicleState_T* state, void* dest, void* value, size_t sz)
{
  if (sz > VEHICLESTATE_QUEUE_MAX_VALUE_SIZE) {
    return VEHCILESTATE_STATUS_ERROR_SIZE;
  }

  VehicleState_QueuedData_T queuedData;
  memcpy(queuedData.data, value, sz);
  queuedData.dataSize = sz;
  queuedData.dest = dest;

  BaseType_t status = xQueueSendToBack(state->dataQueueHandle, (void*)&queuedData, (TickType_t)10U);
  if (pdTRUE != status) {
    return VEHCILESTATE_STATUS_ERROR_QUEUE;
  }

  return VEHICLESTATE_STATUS_OK;
}

//------------------------------------------------------------------------------
VehicleState_Status_T VehicleState_PushFieldf(VehicleState_T* state, float* dest, float value)
{
  VehicleState_QueuedData_T queuedData;
  memcpy(queuedData.data, &value, sizeof(float));
  queuedData.dataSize = sizeof(float);
  queuedData.dest = dest;

  BaseType_t status = xQueueSendToBack(state->dataQueueHandle, (void*)&queuedData, (TickType_t)10U);
  if (pdTRUE != status) {
    return VEHCILESTATE_STATUS_ERROR_QUEUE;
  }

  return VEHICLESTATE_STATUS_OK;
}

//------------------------------------------------------------------------------
bool VehicleState_CopyState(VehicleState_T* state, VehicleState_Data_T* dest)
{
  BaseType_t result = xSemaphoreTake(state->mutex, portMAX_DELAY);
  if (result != pdTRUE) {
    return false;
  }

  memcpy(dest, &state->data, sizeof(VehicleState_Data_T));

  xSemaphoreGive(state->mutex);
  return true;
}

//------------------------------------------------------------------------------
bool VehicleState_AccessAcquire(VehicleState_T* state)
{
  BaseType_t result = xSemaphoreTake(state->mutex, portMAX_DELAY);
  return (result == pdTRUE) ? true : false;
}

//------------------------------------------------------------------------------
bool VehicleState_AccessRelease(VehicleState_T* state)
{
  BaseType_t result = xSemaphoreGive(state->mutex);
  return (result == pdTRUE) ? true : false;
}
