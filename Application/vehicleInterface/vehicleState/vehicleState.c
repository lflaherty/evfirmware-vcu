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

// ------------------- Private methods -------------------
static void StateProcessing(void* pvParameters)
{
  VehicleState_T* obj = (VehicleState_T*)pvParameters;

  const TickType_t blockTime = 100 / portTICK_PERIOD_MS; // 100ms
  uint32_t notifiedValue;

  while (1) {
    // Wait for notification to wake up
    notifiedValue = ulTaskNotifyTake(pdTRUE, blockTime);
    if (notifiedValue > 0) {

      // Acquire lock on data
      if (pdTRUE == xSemaphoreTake(obj->mutex, portMAX_DELAY)) {
        // Process any waiting data
        VehicleState_QueuedData_T queuedData;
        while (pdTRUE == xQueueReceive(obj->dataQueueHandle, &queuedData, 10U)) {

          // copy data out
          switch (queuedData.dataType) {
            case VEHICLESTATE_TYPE_FLOAT: {
              *(float*)queuedData.dest = queuedData.data.dFloat;
              break;
            }
          }

        }

        xSemaphoreGive(obj->mutex);
      }

    }
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
      StateProcessing,
      "ExampleTask",
      VEHCILESTATE_STACK_SIZE,   /* Stack size */
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
VehicleState_Status_T VehicleState_PushFieldf(VehicleState_T* state, void* dest, float value)
{
  VehicleState_QueuedData_T queuedData;
  queuedData.data.dFloat = value;
  queuedData.dataType = VEHICLESTATE_TYPE_FLOAT;
  queuedData.dest = dest;

  BaseType_t status = xQueueSendToBack(state->dataQueueHandle, (void*)&queuedData, (TickType_t)10U);
  if (pdTRUE != status) {
    return VEHCILESTATE_STATUS_ERROR_QUEUE;
  }

  return VEHICLESTATE_STATUS_OK;
}

//------------------------------------------------------------------------------
