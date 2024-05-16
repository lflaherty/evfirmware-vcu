/*
 * discretesense.c
 *
 *  Created on: 21 May 2023
 *      Author: Liam Flaherty
 */
#include "discretesense.h"

#include <stdint.h>
#include "tasktimer/tasktimer.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms

// ------------------- Private methods -------------------
static void DiscreteSense_TaskMethod(DiscreteSense_T* ds)
{
  // Wait for notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    // Sample each and update the vehicle state

    uint16_t rawAccelA;
    uint16_t rawAccelB;
    uint16_t rawBrakeFront;
    uint16_t rawBrakeRear;

    ADC_Status_T retAccelA = ADC_Get(ds->adcAccelPedalA, &rawAccelA);
    ADC_Status_T retAccelB = ADC_Get(ds->adcAccelPedalB, &rawAccelB);
    ADC_Status_T retBrakeFront = ADC_Get(ds->adcBrakeFront, &rawBrakeFront);
    ADC_Status_T retBrakeRear = ADC_Get(ds->adcBrakeRear, &rawBrakeRear);

    float accelA = ADC_ApplyScaling(&ds->scalingAccelPedalA, rawAccelA);
    float accelB = ADC_ApplyScaling(&ds->scalingAccelPedalB, rawAccelB);
    float brakeFront = ADC_ApplyScaling(&ds->scalingBrakeFront, rawBrakeFront);
    float brakeRear = ADC_ApplyScaling(&ds->scalingBrakeRear, rawBrakeRear);

    float accel = 0.5f * (accelA + accelB);

    bool dashButtonPressed = GPIO_ReadPin(ds->gpioDashboardButton);

    if (VehicleState_AccessAcquire(ds->state)) {
      VehicleState_Data_T* stateData = &ds->state->data;

      if (ADC_STATUS_OK == retAccelA) {
        stateData->inputs.accelA = accelA;
        stateData->inputs.accelRawA = rawAccelA;
      }
      if (ADC_STATUS_OK == retAccelB) {
        stateData->inputs.accelB = accelB;
        stateData->inputs.accelRawB = rawAccelB;
      }
      if (ADC_STATUS_OK == retAccelA && ADC_STATUS_OK == retAccelB) {
        stateData->inputs.accel = accel;
        stateData->inputs.accelValid = true;
      }

      if (ADC_STATUS_OK == retBrakeFront) {
        stateData->inputs.brakePresFront = brakeFront;
        stateData->inputs.brakeRawFront = rawBrakeFront;
      }
      if (ADC_STATUS_OK == retBrakeRear) {
        stateData->inputs.brakePresRear = brakeRear;
        stateData->inputs.brakeRawRear = rawBrakeRear;
      }

      stateData->dash.buttonPressed = dashButtonPressed;
    }

    VehicleState_AccessRelease(ds->state);
  }
}

// LCOV_EXCL_START
static void DiscreteSense_Task(void* pvParameters)
{
  DiscreteSense_T* obj = (DiscreteSense_T*)pvParameters;

  while (1) {
    DiscreteSense_TaskMethod(obj);
  }
}
// LCOV_EXCL_STOP

// ------------------- Public methods -------------------
DiscreteSense_Status_T DiscreteSense_Init(DiscreteSense_T* module)
{
  mLog = module->logger;
  Log_Print(mLog, "DiscreteSense_Init begin\n");
  DEPEND_ON(module->logger, DISCRETESENSE_STATUS_ERROR_DEPENDS);
  DEPEND_ON(module->state, DISCRETESENSE_STATUS_ERROR_DEPENDS);
  DEPEND_ON_STATIC(ADC, DISCRETESENSE_STATUS_ERROR_DEPENDS);
  DEPEND_ON_STATIC(TASKTIMER, DISCRETESENSE_STATUS_ERROR_DEPENDS);

  if (NULL == module->gpioDashboardButton) {
    return DISCRETESENSE_STATUS_ERROR_INIT;
  }

  // create main task
  module->taskHandle = xTaskCreateStatic(
      DiscreteSense_Task,
      "DiscreteSense",
      DISCRETESENSE_STACK_SIZE,
      (void*)module,  /* Parameter passed as pointer */
      DISCRETESENSE_TASK_PRIORITY,
      module->taskStack,
      &module->taskBuffer);

  // Register the task for timer notifications every 10ms (100Hz)
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(
      &module->taskHandle,
      TASKTIMER_FREQUENCY_100HZ);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return DISCRETESENSE_STATUS_ERROR_INIT;
  }

  REGISTER(module, DISCRETESENSE_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "DiscreteSense_Init complete\n");
  return DISCRETESENSE_STATUS_OK;
}
