/*
 * watchdogTrigger.c
 *
 *  Created on: Jul 31 2022
 *      Author: Liam Flaherty
 */

#include "watchdogTrigger.h"

#include "time/tasktimer/tasktimer.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms
static const uint32_t tickRateMs = 10U;

// ------------------- Private methods -------------------
static void WatchdogTrigger(WatchdogTrigger_T* wdtTrigger)
{
  // Wait for notification to wake up
  uint32_t notifiedValue = ulTaskNotifyTake(pdTRUE, mBlockTime);
  if (notifiedValue > 0) {
    // TODO notify WDT

    // status LED blinking
    wdtTrigger->counter++;
    if (100U == wdtTrigger->counter) {
      GPIO_TogglePin(wdtTrigger->blinkLED);
      wdtTrigger->counter = 0U;
    }
  }
}

static void WatchdogTrigger_Task(void* pvParameters)
{
  WatchdogTrigger_T* obj = (WatchdogTrigger_T*)pvParameters;

  while (1) {
    WatchdogTrigger(obj);
  }
}

// ------------------- Public methods -------------------
WatchdogTrigger_Status_T WatchdogTrigger_Init(
    Logging_T* logger,
    WatchdogTrigger_T* wdtTrigger)
{
  mLog = logger;
  Log_Print(mLog, "WatchdogTrigger_Init begin\n");
  DEPEND_ON(logger, WATCHDOGTRIGGER_STATUS_ERROR_DEPENDS);
  DEPEND_ON_STATIC(TASKTIMER, WATCHDOGTRIGGER_STATUS_ERROR_DEPENDS);

  wdtTrigger->counter = 0U;

  // Create mutex lock
  wdtTrigger->mutex = xSemaphoreCreateMutexStatic(&wdtTrigger->mutexBuffer);

  // create main task
  wdtTrigger->taskHandle = xTaskCreateStatic(
      WatchdogTrigger_Task,
      "WatchdogTrigger",
      WATCHDOGTRIGGER_STACK_SIZE,
      (void*)wdtTrigger,  /* Parameter passed as pointer */
      WATCHDOGTRIGGER_TASK_PRIORITY,
      wdtTrigger->taskStack,
      &wdtTrigger->taskBuffer);

  // Register the task for timer notifications every 10ms (100Hz)
  uint16_t timerDivider = tickRateMs * TASKTIMER_BASE_PERIOD_MS;
  TaskTimer_Status_T statusTimer = TaskTimer_RegisterTask(&wdtTrigger->taskHandle, timerDivider);
  if (TASKTIMER_STATUS_OK != statusTimer) {
    return WATCHDOGTRIGGER_STATUS_ERROR_INIT;
  }

  REGISTER(wdtTrigger, WATCHDOGTRIGGER_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "WatchdogTrigger_Init complete\n");
  return WATCHDOGTRIGGER_STATUS_OK;
}
