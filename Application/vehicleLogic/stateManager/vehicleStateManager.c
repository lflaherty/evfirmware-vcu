/*
 * vehicleStateManager.c
 *
 *  Created on: May 11 2022
 *      Author: Liam Flaherty
 */

#include "vehicleStateManager.h"

#include "time/tasktimer/tasktimer.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms
static const uint32_t tickRateMs = 10U;

// ------------------- Private methods -------------------
static void StateManagerProcessing(VehicleStateManager_T* sm)
{
  // Wait for notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    // Run state machine
    VSM_Step(&sm->vsm);
  }
}

static void StateManagerProcessing_Task(void* pvParameters)
{
  VehicleStateManager_T* obj = (VehicleStateManager_T*)pvParameters;

  while (1) {
    StateManagerProcessing(obj);
  }
}

// ------------------- Public methods -------------------
VehicleStateManager_Status_T VehicleStateManager_Init(Logging_T* logger, VehicleStateManager_T* sm)
{
  mLog = logger;
  Log_Print(mLog, "VehicleStateManager_Init begin\n");
  DEPEND_ON_STATIC(TASKTIMER, STATEMANAGER_STATUS_ERROR_DEPENDS);

  sm->vsm.inputState = sm->inputState;
  sm->vsm.control = sm->control;
  sm->vsm.vehicleConfig = sm->vehicleConfig;
  sm->vsm.throttleController = sm->throttleController;
  sm->vsm.tickRateMs = tickRateMs;
  VSM_Init(logger, &sm->vsm);

  // create main task
  sm->taskHandle = xTaskCreateStatic(
      StateManagerProcessing_Task,
      "VehicleStateManager",
      VEHICLESTATEMANAGER_STACK_SIZE,   /* Stack size */
      (void*)sm,  /* Parameter passed as pointer */
      VEHICLESTATEMANAGER_TASK_PRIORITY,
      sm->taskStack,
      &sm->taskBuffer);

  // Register the task for timer notifications every 10ms (100Hz)
  uint16_t timerDivider = tickRateMs * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&sm->taskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return STATEMANAGER_STATUS_ERROR_INIT;
  }

  REGISTER(sm, STATEMANAGER_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "VehicleStateManager_Init complete\n");
  return STATEMANAGER_STATUS_OK;
}
