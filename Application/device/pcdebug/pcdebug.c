/*
 * pcdebug.c
 *
 *  Created on: Aug 2 2022
 *      Author: Liam Flaherty
 */
#include "pcdebug.h"

#include "time/tasktimer/tasktimer.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms
static const uint32_t tickRateMs = 10U;

// ------------------- Private methods -------------------
static void PCDebug_TaskMethod(PCDebug_T* pcdebug)
{
  // Wait for notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    // TODO run task
    (void)pcdebug;
  }
}

static void PCDebug_Task(void* pvParameters)
{
  PCDebug_T* obj = (PCDebug_T*)pvParameters;

  while (1) {
    PCDebug_TaskMethod(obj);
  }
}

// ------------------- Public methods -------------------
PCDebug_Status_T PCDebug_Init(
    Logging_T* logger,
    PCDebug_T* pcdebug)
{
  mLog = logger;
  logPrintS(mLog, "PCDebug_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // Create mutex lock
  pcdebug->mutex = xSemaphoreCreateBinaryStatic(&pcdebug->mutexBuffer);

  // create main task
  pcdebug->taskHandle = xTaskCreateStatic(
      PCDebug_Task,
      "PCDebug",
      PCDEBUG_STACK_SIZE,
      (void*)pcdebug,  /* Parameter passed as pointer */
      PCDEBUG_TASK_PRIORITY,
      pcdebug->taskStack,
      &pcdebug->taskBuffer);

  // Register the task for timer notifications every 10ms (100Hz)
  uint16_t timerDivider = tickRateMs * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&pcdebug->taskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return PCDEBUG_STATUS_ERROR_INIT;
  }

  // Register to receive logging data

  logPrintS(mLog, "PCDebug_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return PCDEBUG_STATUS_OK;
}
