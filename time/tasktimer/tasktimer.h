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

/*
 * The base tick rate. This can be divided for individual tasks.
 * Note that this is configured in main with the TIM_HandleTypeDef Prescaler and Period.
 * Duplicated here in units of MS.
 * Change this if the Prescaler and Period are updated.
 */
#define TASKTIMER_BASE_PERIOD_MS 1U

/*
 * Max number of tasks allowed to be registered
 */
#define TASKTIMER_MAX_TASKS 16U

typedef enum {
  TASKTIMER_STATUS_OK                     = 0x00U,
  TASKTIMER_STATUS_ERROR_TIMER            = 0x01U,
  TASKTIMER_STATUS_ERROR_FULL             = 0x02U,
  TASKTIMER_STATUS_ERROR_INVALID_DIVIDER  = 0x03U
} TaskTimer_Status_T;

/*
 * Starts the timer function
 * @param htim Handle for the timer to be used.
 */
TaskTimer_Status_T TaskTimer_Init(TIM_HandleTypeDef* htim);

/*
 * Registers a task for real-time notifications.
 * When the timer has elapsed, a notification will be sent to the task
 *
 * @param task The task to be notified
 * @param divider Notify every n timer ticks. Set as 1 to notify for every tick.
 * E.g. if divider=5 for a 1ms clock, only run once every 5 ticks (i.e. 5ms)
 */
TaskTimer_Status_T TaskTimer_RegisterTask(TaskHandle_t* task, uint16_t divider);

/*
 * Handler for the timer period elapsed callback.
 * Invoke this method from the main HAL_TIM_PeriodElapsedCallback
 *
 * This is an ISR method.
 */
void TaskTimer_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim);

#endif /* TIME_TASKTIMER_TASKTIMER_H_ */
