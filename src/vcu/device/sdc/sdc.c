/*
 * sdc.c
 *
 *  Created on: 13 Nov 2022
 *      Author: Liam Flaherty
 */

#include "sdc.h"

#include <string.h>
#include "FreeRTOS.h"
#include "task.h"

REGISTERED_MODULE_STATIC_DEF(SDC);

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 10 / portTICK_PERIOD_MS; // 10ms

static struct
{
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[SDC_STACK_SIZE];
} mTask;

static SDC_Config_T mConfig;

volatile static VehicleState_SDC_T mSDCInputs;

// ------------------- Private methods -------------------
static void SDC_TaskMethod(void)
{
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    // A notification means the IRQ handler collected new data

    if (VehicleState_AccessAcquire(mConfig.state)) {
      mConfig.state->data.vehicle.sdc = mSDCInputs;
      VehicleState_AccessRelease(mConfig.state);
    }
  }
}

static void SDC_Task(void* pvParameters)
{
  (void)pvParameters;

  while (1) {
    SDC_TaskMethod();
  }
}

/**
 * @brief Update an internal GPIO input state variable from IRQ.
 * 
 * @param currentVal Pointer to current value to check and update.
 * @param pin Pin for that value.
 * @return true if a value was changed and a notification should be made
 */
static bool updateSdcInputIRQ(volatile bool* currentVal, GPIO_T* pin)
{
  bool newVal = GPIO_ReadPin(pin);
  if (*currentVal != newVal) {
    *currentVal = newVal;
    return true;
  }

  return false;
}

// ------------------- Public methods -------------------
SDC_Status_T SDC_Init(Logging_T* logger, SDC_Config_T* config)
{
  mLog = logger;
  Log_Print(mLog, "SDC_Init begin\n");
  DEPEND_ON(logger, SDC_STATUS_ERROR_DEPENDS);
  DEPEND_ON(config->state, SDC_STATUS_ERROR_DEPENDS);

  mSDCInputs.bms = false; // Note: can't memset this because volatile
  mSDCInputs.bspd = false;
  mSDCInputs.imd = false;
  mSDCInputs.out = false;
  mConfig = *config;

  // create main task
  mTask.taskHandle = xTaskCreateStatic(
      SDC_Task,
      "SDC",
      SDC_STACK_SIZE,
      NULL,  /* Parameter passed as pointer */
      SDC_TASK_PRIORITY,
      mTask.taskStack,
      &mTask.taskBuffer);

  REGISTER_STATIC(SDC, SDC_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "SDC_Init complete\n");
  return SDC_STATUS_OK;
}

SDC_Status_T SDC_AssertECUFault(const bool fault)
{
  GPIO_WritePin(mConfig.pinECUError, fault);
  return SDC_STATUS_OK;
}

void SDC_IRQHandler(uint16_t pin)
{
  (void)pin;

  bool notify = false;

  notify |= updateSdcInputIRQ(&mSDCInputs.bms, mConfig.pinBMS);
  notify |= updateSdcInputIRQ(&mSDCInputs.bspd, mConfig.pinBSPD);
  notify |= updateSdcInputIRQ(&mSDCInputs.imd, mConfig.pinIMD);
  notify |= updateSdcInputIRQ(&mSDCInputs.out, mConfig.pinSDCOut);

  if (notify) {
    vTaskNotifyGiveFromISR(mTask.taskHandle, NULL);
  }
}
