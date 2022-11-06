/*
 * throttleController.h
 *
 *  Created on: Jun 19 2022
 *      Author: Liam Flaherty
 */

#include "throttleController.h"

#include <math.h>

#include "time/tasktimer/tasktimer.h"
#include "torqueMap.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms
static const uint32_t tickRateMs = 10U;

// ------------------- Private methods -------------------
static float getTorqueMagnitude(
    ThrottleController_T* throttleControl,
    float accelPedal)
{
  if (throttleControl->enabled == false) {
    return 0.0f;
  }

  TorqueMap_T* torqueMap = NULL;

  if (VEHICLESTATE_INVERTER_FORWARD == throttleControl->direction) {
    torqueMap = throttleControl->torqueMapForward;
  } else if (VEHICLESTATE_INVERTER_REVERSE == throttleControl->direction) {
    torqueMap = throttleControl->torqueMapReverse;
  } else {
    return 0.0f;
  }

  float torque = TorqueMap_Interpolate(torqueMap, accelPedal);

  return torque;
}

static void ThrottleController(ThrottleController_T* throttleControl)
{
  // Wait for notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    // Acquire data
    float accelPedal = 0.0f;
    bool accessData = VehicleState_AccessAcquire(throttleControl->inputState);
    if (accessData) {
      accelPedal = throttleControl->inputState->data.inputs.accel;
      VehicleState_AccessRelease(throttleControl->inputState);
    }

    // Determine torque required
    float torqueCommand = getTorqueMagnitude(throttleControl, accelPedal);

    // Send torque instruction
    VehicleControl_RequestMotorTorque(
        throttleControl->control,
        torqueCommand,
        throttleControl->direction);
  }
}

static void ThrottleController_Task(void* pvParameters)
{
  ThrottleController_T* obj = (ThrottleController_T*)pvParameters;

  while (1) {
    ThrottleController(obj);
  }
}

// ------------------- Public methods -------------------
ThrottleController_Status_T ThrottleController_Init(
    Logging_T* logger,
    ThrottleController_T* throttleControl)
{
  mLog = logger;
  Log_Print(mLog, "ThrottleController_Init begin\n");

  throttleControl->torqueMapForward = &TorqueMap_Default;
  throttleControl->torqueMapReverse = &TorqueMap_DefaultReverse;
  throttleControl->enabled = false;
  throttleControl->direction = VEHICLESTATE_INVERTER_FORWARD;

  // Create mutex lock
  throttleControl->mutex = xSemaphoreCreateMutexStatic(&throttleControl->mutexBuffer);

  // create main task
  throttleControl->taskHandle = xTaskCreateStatic(
      ThrottleController_Task,
      "ThrottleController",
      THROTTLECONTROLLER_STACK_SIZE,
      (void*)throttleControl,  /* Parameter passed as pointer */
      THROTTLECONTROLLER_TASK_PRIORITY,
      throttleControl->taskStack,
      &throttleControl->taskBuffer);

  // Register the task for timer notifications every 10ms (100Hz)
  uint16_t timerDivider = tickRateMs * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&throttleControl->taskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return THROTTLECONTROLLER_STATUS_ERROR_INIT;
  }

  Log_Print(mLog, "ThrottleController_Init complete\n");
  return THROTTLECONTROLLER_STATUS_OK;
}


ThrottleController_Status_T ThrottleController_SetTorqueEnabled(
    ThrottleController_T* throttleControl,
    bool enabled)
{
  BaseType_t result = xSemaphoreTake(throttleControl->mutex, portMAX_DELAY);
  if (result != pdTRUE) {
    return THROTTLECONTROLLER_STATUS_ERROR_MUTEX;
  }

  throttleControl->enabled = enabled;

  xSemaphoreGive(throttleControl->mutex);
  return THROTTLECONTROLLER_STATUS_OK;
}

ThrottleController_Status_T ThrottleController_SetMotorDirection(
    ThrottleController_T* throttleControl,
    VehicleState_InverterDirection_T direction)
{
  BaseType_t result = xSemaphoreTake(throttleControl->mutex, portMAX_DELAY);
  if (result != pdTRUE) {
    return THROTTLECONTROLLER_STATUS_ERROR_MUTEX;
  }

  throttleControl->direction = direction;

  xSemaphoreGive(throttleControl->mutex);
  return THROTTLECONTROLLER_STATUS_OK;
}
