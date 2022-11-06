/*
 * MockTasktimer.c
 *
 *  Created on: Oct 17 2021
 *      Author: Liam Flaherty
 */

#include "MockTasktimer.h"

// ------------------- Static data -------------------
static TaskTimer_Status_T mStatus_TaskTimer_Init = TASKTIMER_STATUS_OK;
static TaskTimer_Status_T mStatus_TaskTimer_RegisterTask = TASKTIMER_STATUS_OK;

// ------------------- Methods -------------------
TaskTimer_Status_T TaskTimer_Init(Logging_T* logger, TIM_HandleTypeDef* htim)
{
    (void)logger;
    (void)htim;
    return mStatus_TaskTimer_Init;
}

TaskTimer_Status_T TaskTimer_RegisterTask(TaskHandle_t* task, uint32_t divider)
{
    (void)task;
    (void)divider;
    return mStatus_TaskTimer_RegisterTask;
}

void TaskTimer_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim)
{
    (void)htim;
}

void mockSet_TaskTimer_Init_Status(TaskTimer_Status_T status)
{
    mStatus_TaskTimer_Init = status;
}

void mockSet_TaskTimer_RegisterTask_Status(TaskTimer_Status_T status)
{
    mStatus_TaskTimer_RegisterTask = status;
}
