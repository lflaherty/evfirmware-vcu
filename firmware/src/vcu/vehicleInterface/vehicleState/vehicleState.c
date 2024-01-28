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

// ------------------- Public methods -------------------
VehicleState_Status_T VehicleState_Init(Logging_T* logger, VehicleState_T* state)
{
  mLog = logger;
  Log_Print(mLog, "VehicleState_Init begin\n");
  DEPEND_ON_STATIC(TASKTIMER, VEHICLESTATE_STATUS_ERROR_DEPENDS);

  memset(&state->data, 0, sizeof(state->data)); // initialize data to 0

  // Create mutex lock
  state->mutex = xSemaphoreCreateMutexStatic(&state->mutexBuffer);

  REGISTER(state, VEHICLESTATE_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "VehicleState_Init complete\n");
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
