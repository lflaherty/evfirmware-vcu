/*
 * tasktimer.c
 *
 *  Created on: 6 May 2021
 *      Author: Liam Flaherty
 */

#include "tasktimer.h"

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#include "lib/depends/depends.h"
#include "lib/logging/logging.h"

REGISTERED_MODULE_STATIC_DEF(TASKTIMER);

// ------------------- Static data ----------------------
static Logging_T* mLog;

typedef struct {
  bool isSet;                  // Only true when used
  TaskHandle_t* taskHandle;    // Handle for notification
} TimedTask_T;

typedef struct {
  uint8_t numTasks;
  TimedTask_T tasks[TASKTIMER_MAX_TASKS];
} TaskList_T;

static TIM_HandleTypeDef* timHandles[TASKTIMER_FREQUENCY_COUNT] = {0};
static TaskList_T taskLists[TASKTIMER_FREQUENCY_COUNT] = {0};
static bool isInitialized = false;


// ------------------- Public methods -------------------
TaskTimer_Status_T TaskTimer_Init(
    Logging_T* logger,
    TIM_HandleTypeDef* htim100Hz)
{
  mLog = logger;
  Log_Print(mLog, "TaskTimer_Init begin\n");
  DEPEND_ON(logger, TASKTIMER_STATUS_ERROR_DEPENDS);

  memset(&taskLists, 0U, sizeof(taskLists));
  memset(&timHandles, 0U, sizeof(timHandles));

  timHandles[TASKTIMER_FREQUENCY_100HZ] = htim100Hz;
  isInitialized = true;

  // Start the timers
  for (uint16_t i = 0; i < TASKTIMER_FREQUENCY_COUNT; ++i) {
    if (NULL == timHandles[i] || HAL_OK != HAL_TIM_Base_Start_IT(timHandles[i])) {
      return TASKTIMER_STATUS_ERROR_TIMER;
    }
  }

  REGISTER_STATIC(TASKTIMER, TASKTIMER_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "TaskTimer_Init complete\n");
  return TASKTIMER_STATUS_OK;
}

//------------------------------------------------------------------------------
TaskTimer_Status_T TaskTimer_RegisterTask(
    TaskHandle_t* task,
    const TaskTimer_Frequency_T timer)
{
  DEPEND_ON_STATIC(TASKTIMER, TASKTIMER_STATUS_ERROR_DEPENDS);

  if (timer >= TASKTIMER_FREQUENCY_COUNT) {
    return TASKTIMER_STATUS_ERROR_INVALID_TIMER;
  }

  // Check size of task list
  if (TASKTIMER_MAX_TASKS == taskLists[timer].numTasks) {
    // Timer list is full
    return TASKTIMER_STATUS_ERROR_FULL;
  }

  // Store task
  uint8_t i = taskLists[timer].numTasks;
  taskLists[timer].tasks[i].taskHandle = task;
  taskLists[timer].tasks[i].isSet = true;
  taskLists[timer].numTasks++;

  return TASKTIMER_STATUS_OK;
}

//------------------------------------------------------------------------------
void TaskTimer_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
  if (!isInitialized)
  {
    return;
  }

  TaskList_T* taskList = NULL;

  // Find the relevant task list
  for (uint16_t i = 0; i < TASKTIMER_FREQUENCY_COUNT; ++i) {
    if (htim->Instance == timHandles[i]->Instance) {
      taskList = &taskLists[i];
      break;
    }
  }

  if (NULL == taskList) {
    return;
  }

  // Notify all tasks in the list
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  for (uint16_t i = 0; i < taskList->numTasks; ++i) {
    BaseType_t taskWokenI = pdFALSE;
    vTaskNotifyGiveFromISR(*taskList->tasks[i].taskHandle, &taskWokenI);

    higherPriorityTaskWoken = (taskWokenI == pdTRUE) ? pdTRUE : higherPriorityTaskWoken;
  }

  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}
