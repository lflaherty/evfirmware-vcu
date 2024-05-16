/*
 * tasktimer.h
 *
 * Provides notifications to tasks using hardware timers.
 *
 *  Created on: 6 May 2021
 *      Author: Liam Flaherty
 */

#ifndef TIME_TASKTIMER_TASKTIMER_H_
#define TIME_TASKTIMER_TASKTIMER_H_

#include <stdint.h>
#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

#include "depends/depends.h"
#include "logging/logging.h"

REGISTERED_MODULE_STATIC(TASKTIMER);

/*
 * Max number of tasks allowed to be registered
 */
#define TASKTIMER_MAX_TASKS 16U

typedef enum {
  TASKTIMER_STATUS_OK                     = 0x00U,
  TASKTIMER_STATUS_ERROR_TIMER            = 0x01U,
  TASKTIMER_STATUS_ERROR_FULL             = 0x02U,
  TASKTIMER_STATUS_ERROR_INVALID_TIMER    = 0x03U,
  TASKTIMER_STATUS_ERROR_DEPENDS          = 0x04U,
} TaskTimer_Status_T;

typedef enum {
  TASKTIMER_FREQUENCY_100HZ = 0U,
  TASKTIMER_FREQUENCY_COUNT,
} TaskTimer_Frequency_T;

/*
 * Starts the timer function
 * @param logger Pointer to logging settings
 * @param htim100Hz Handle for the 100Hz timer
 */
TaskTimer_Status_T TaskTimer_Init(
    Logging_T* logger,
    TIM_HandleTypeDef* htim100Hz);

/*
 * Registers a task for real-time notifications.
 * When the timer has elapsed, a notification will be sent to the task
 * 
 * Various timers are set up to trigger at different frequencies.
 * This method will internally map TaskTimer_Frequency_T to the correct
 * hardware timer.
 *
 * @param task The task to be notified
 * @param timer The timer to notify from.
 */
TaskTimer_Status_T TaskTimer_RegisterTask(TaskHandle_t* task, const TaskTimer_Frequency_T timer);

/*
 * Handler for the timer period elapsed callback.
 * Invoke this method from the main HAL_TIM_PeriodElapsedCallback
 *
 * This is an ISR method.
 */
void TaskTimer_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim);

#endif /* TIME_TASKTIMER_TASKTIMER_H_ */
