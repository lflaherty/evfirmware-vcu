/*
 * mockQueue.c
 * Mock implementation for FreeRTOS queue.h
 * 
 *  Created on: Oct 10, 2021
 *      Author: Liam Flaherty
 */

#include "FreeRTOS.h"
#include "queue.h"

#include <string.h>

// ------------------- Static data -------------------
static const void* mQueueData; // single data entry in queue

// ------------------- Methods -------------------
QueueHandle_t xQueueCreateStatic(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, uint8_t* pucQueueStorage, StaticQueue_t* pxStaticQueue)
{
    (void)uxQueueLength;
    (void)pucQueueStorage;

    QueueHandle_t handle = (QueueHandle_t)pxStaticQueue;
    handle->itemSize = uxItemSize;

    return handle;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void* const pvBuffer, TickType_t xTicksToWait)
{
    (void)xTicksToWait;

    memcpy(pvBuffer, mQueueData, xQueue->itemSize);

    return pdPASS;
}

BaseType_t xQueueSendToBackFromISR(QueueHandle_t xQueue, const void* const pvItemToQueue, BaseType_t* const pxHigherPriorityTaskWoken)
{
    (void)xQueue;
    (void)pxHigherPriorityTaskWoken;

    mQueueData = pvItemToQueue;

    return pdPASS;
}

void mockSetQueueDataPtr(void* data)
{
    mQueueData = data;
}