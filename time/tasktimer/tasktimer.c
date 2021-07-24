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

// ------------------- Static data ----------------------
typedef struct {
  bool isSet;                  // Only true when used
  TaskHandle_t* taskHandle;    // Handle for notification
  uint16_t divider;            // Timer divider. How many timer ticks to count.
  uint16_t timerCounter;       // Current count of timer ticks.
} TimedTask_T;

static struct {
  uint8_t numTasks;
  TimedTask_T tasks[TASKTIMER_MAX_TASKS];
} taskData;

static TIM_HandleTypeDef* timHandle;


// ------------------- Public methods -------------------
TaskTimer_Status_T TaskTimer_Init(TIM_HandleTypeDef* htim)
{
  printf("TaskTimer_Init begin\n");

  // Init data
  memset(&taskData, 0, sizeof(taskData));
  timHandle = htim;

  // Just start the timer
  if (HAL_OK != HAL_TIM_Base_Start_IT(htim)) {
    return TASKTIMER_STATUS_ERROR_TIMER;
  }

  printf("TaskTimer_Init complete\n");
  return TASKTIMER_STATUS_OK;
}

//------------------------------------------------------------------------------
TaskTimer_Status_T TaskTimer_RegisterTask(TaskHandle_t* task, uint16_t divider)
{
  // Check size of task list
  if (TASKTIMER_MAX_TASKS == taskData.numTasks) {
    // Timer list is full
    return TASKTIMER_STATUS_ERROR_FULL;
  }

  // Check divider value
  if (0 == divider) {
    // Invalid divider value
    return TASKTIMER_STATUS_ERROR_INVALID_DIVIDER;
  }

  // Store task
  taskData.tasks[taskData.numTasks].taskHandle = task;
  taskData.tasks[taskData.numTasks].divider = divider;
  taskData.tasks[taskData.numTasks].timerCounter = 1; // timer always counts up from 1
  taskData.tasks[taskData.numTasks].isSet = true;
  taskData.numTasks++;

  return TASKTIMER_STATUS_OK;
}

//------------------------------------------------------------------------------
void TaskTimer_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
  // Check that this is the right timer
  if (htim->Instance == timHandle->Instance) {

    // Determine what notifications to send
    size_t i;
    for (i = 0; i < taskData.numTasks; ++i) {
      // Check that this is in use
      if (taskData.tasks[i].isSet && NULL != taskData.tasks[i].taskHandle) {

        // Check timer divider
        if (taskData.tasks[i].divider == taskData.tasks[i].timerCounter) {
          // Ready for notification
          vTaskNotifyGiveFromISR(*taskData.tasks[i].taskHandle, NULL);

          // Reset counter
          taskData.tasks[i].timerCounter = 1;
        } else {
          // Increment the counter
          taskData.tasks[i].timerCounter++;
        }

      }
    }
  }
}
