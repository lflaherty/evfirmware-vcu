/*
 * mockQueue.c
 * Mock implementation for FreeRTOS queue.h
 * 
 *  Created on: Oct 10, 2021
 *      Author: Liam Flaherty
 */

#include "FreeRTOS.h"
#include "queue.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

// ------------------- Methods -------------------
QueueHandle_t xQueueCreateStatic(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, uint8_t* pucQueueStorage, StaticQueue_t* pxStaticQueue)
{
    (void)uxQueueLength;
    (void)pucQueueStorage;

    QueueHandle_t handle = (QueueHandle_t)pxStaticQueue;
    handle->itemSize = uxItemSize;
    handle->queueLen = uxQueueLength;
    handle->start = 0U;
    handle->end = 0U;

    return handle;
}

BaseType_t xQueueReceive(QueueHandle_t xQueue, void* const pvBuffer, TickType_t xTicksToWait)
{
    (void)xTicksToWait;

    size_t remainingSize = mockGetQueueSize(xQueue);
    if (remainingSize < xQueue->itemSize) {
        return pdFALSE;
    }

    memcpy(pvBuffer, xQueue->data + xQueue->start, xQueue->itemSize);
    xQueue->start += xQueue->itemSize;

    return pdTRUE;
}

BaseType_t xQueueReceiveFromISR(QueueHandle_t xQueue, void *pvBuffer, BaseType_t *pxHigherPriorityTaskWoken)
{
    *pxHigherPriorityTaskWoken = pdFALSE;
    return xQueueReceive(xQueue, pvBuffer, 0U);
}

BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void* const pvItemToQueue, TickType_t ticksToWait)
{
    (void)ticksToWait;
    assert(xQueue != NULL);

    size_t freeSpace = MOCK_QUEUE_SIZE - xQueue->end;
    assert(freeSpace >= xQueue->itemSize);
    assert(xQueue->queueLen > 0);

    size_t queueNumElements = mockGetQueueSize(xQueue) / xQueue->itemSize;
    if (queueNumElements >= xQueue->queueLen) {
        return pdFALSE;
    }

    memcpy(xQueue->data + xQueue->end, pvItemToQueue, xQueue->itemSize);
    xQueue->end += xQueue->itemSize;

    return pdTRUE;
}

BaseType_t xQueueSendToBackFromISR(QueueHandle_t xQueue, const void* const pvItemToQueue, BaseType_t* const pxHigherPriorityTaskWoken)
{
    (void)pxHigherPriorityTaskWoken;

    return xQueueSendToBack(xQueue, pvItemToQueue, 0U);
}

BaseType_t xQueueIsQueueEmptyFromISR(const QueueHandle_t xQueue)
{
    return xQueue->start == xQueue->end;
}

void mockClearQueueData(QueueHandle_t xQueue)
{
    xQueue->start = 0U;
    xQueue->end = 0U;
}

void mockSetQueueData(QueueHandle_t xQueue, void* data, size_t dataSize)
{
    assert(dataSize <= MOCK_QUEUE_SIZE);

    xQueue->start = 0;
    xQueue->end = dataSize;
    memcpy(xQueue->data, data, dataSize);
}

size_t mockGetQueueSize(QueueHandle_t xQueue)
{
    return xQueue->end - xQueue->start;
}

bool mockGetQueueData(QueueHandle_t xQueue, void* data, size_t dataSize)
{
    assert(mockGetQueueSize(xQueue) == dataSize);

    memcpy(data, xQueue->data, dataSize);
    return true;
}
