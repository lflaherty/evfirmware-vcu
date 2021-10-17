/**
 * MockTaskTimer.c
 * 
 *  Created on: Oct 17 2021
 *      Author: Liam Flaherty
 */

#include "MockTaskTimer.h"

// ------------------- Static data ----------------------
static TaskTimer_Status_T mInitStatus = TASKTIMER_STATUS_OK;
static TaskTimer_Status_T mRegisterTaskStatus = TASKTIMER_STATUS_OK;

// ------------------- Methods -------------------
TaskTimer_Status_T TaskTimer_Init(Logging_T* logger, TIM_HandleTypeDef* htim)
{
    (void)logger;
    (void)htim;
    return mInitStatus;
}

TaskTimer_Status_T TaskTimer_RegisterTask(TaskHandle_t* task, uint32_t divider)
{
    (void)task;
    (void)divider;
    return mRegisterTaskStatus;
}

void mockSet_TaskTimer_Init_Status(TaskTimer_Status_T status)
{
    mInitStatus = status;
}

void mockSet_TaskTimer_RegisterTask_Status(TaskTimer_Status_T status)
{
    mRegisterTaskStatus = status;
}
