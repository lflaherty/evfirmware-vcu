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
#include <assert.h>
#include <stdbool.h>

// ------------------- Static data -------------------
#define MOCK_QUEUE_SIZE 8192 /* something large enough to put anything from the tests in */
static uint8_t mQueueData[MOCK_QUEUE_SIZE]; // single data entry in queue
static size_t start = 0;
static size_t end = 0;

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

    size_t remainingSize = end - start;
    if (remainingSize < xQueue->itemSize) {
        return pdFAIL;
    }

    memcpy(pvBuffer, mQueueData + start, xQueue->itemSize);
    start += xQueue->itemSize;

    return pdPASS;
}

BaseType_t xQueueReceiveFromISR(QueueHandle_t xQueue, void *pvBuffer, BaseType_t *pxHigherPriorityTaskWoken)
{
    *pxHigherPriorityTaskWoken = pdFALSE;
    return xQueueReceive(xQueue, pvBuffer, 0U);
}

BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void* const pvItemToQueue, TickType_t ticksToWait)
{
    (void)ticksToWait;
    
    size_t freeSpace = MOCK_QUEUE_SIZE - end;
    assert(freeSpace >= xQueue->itemSize);

    memcpy(mQueueData + end, pvItemToQueue, xQueue->itemSize);
    end += xQueue->itemSize;

    return pdPASS;
}

BaseType_t xQueueSendToBackFromISR(QueueHandle_t xQueue, const void* const pvItemToQueue, BaseType_t* const pxHigherPriorityTaskWoken)
{
    (void)pxHigherPriorityTaskWoken;

    return xQueueSendToBack(xQueue, pvItemToQueue, 0U);
}

BaseType_t xQueueIsQueueEmptyFromISR(const QueueHandle_t xQueue)
{
    (void)xQueue;
    return start == end;
}

void mockClearQueueData(void)
{
    start = 0U;
    end = 0U;
}

void mockSetQueueData(void* data, size_t dataSize)
{
    assert(dataSize <= MOCK_QUEUE_SIZE);

    start = 0;
    end = dataSize;
    memcpy(mQueueData, data, dataSize);
}

size_t mockGetQueueSize(void)
{
    return end - start;
}

bool mockGetQueueData(void* data, size_t dataSize)
{
    assert(mockGetQueueSize() == dataSize);

    memcpy(data, mQueueData, dataSize);
    return true;
}