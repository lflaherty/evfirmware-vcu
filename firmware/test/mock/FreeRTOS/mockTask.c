/*
 * mockQueue.c
 * Mock implementation for FreeRTOS queue.h
 * 
 *  Created on: Oct 10, 2021
 *      Author: Liam Flaherty
 */

#include "FreeRTOS.h"
#include "task.h"

#include <string.h>

// ------------------- Static data -------------------
static uint32_t mNotifyValue = 0;

// ------------------- Methods -------------------
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                                const char* const pcName,
                                const uint32_t ulStackDepth,
                                void* const pvParameters,
                                UBaseType_t uxPriority,
                                StackType_t* const puxStackBuffer,
                                StaticTask_t* const pxTaskBuffer)
{
    (void)pxTaskCode;
    (void)pcName;
    (void)ulStackDepth;
    (void)pvParameters;
    (void)uxPriority;
    (void)puxStackBuffer;
    (void)pxTaskBuffer;

    TaskHandle_t handle = (TaskHandle_t)pxTaskBuffer;
    return handle;
}

void vTaskNotifyGiveFromISR(TaskHandle_t xTaskToNotify, BaseType_t* pxHigherPriorityTaskWoken)
{
    (void)xTaskToNotify;
    (void)pxHigherPriorityTaskWoken;
    mNotifyValue++;
}

uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit, TickType_t xTicksToWait)
{
    (void)xTicksToWait; // ignore variable

    uint32_t retValue = mNotifyValue;

    if (pdTRUE == xClearCountOnExit) {
        mNotifyValue = 0;
    } else {
        mNotifyValue--;
    }

    return retValue;
}

void mockSetTaskNotifyValue(uint32_t value)
{
    mNotifyValue = value;
}

uint32_t mockGetTaskNotifyValue(void)
{
    return mNotifyValue;
}
