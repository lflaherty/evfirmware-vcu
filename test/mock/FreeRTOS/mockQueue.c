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
#define MOCK_QUEUE_ITEM_SIZE 8192 /* something large enough to put anything from the tests in */
static uint8_t mQueueData[MOCK_QUEUE_ITEM_SIZE]; // single data entry in queue
size_t size = 0;
bool filled = false;

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

    if (!filled) {
        return pdFAIL;
    }

    assert(size == xQueue->itemSize);
    memcpy(pvBuffer, mQueueData, xQueue->itemSize);
    filled = false;

    return pdPASS;
}

BaseType_t xQueueSendToBack(QueueHandle_t xQueue, const void* const pvItemToQueue, TickType_t ticksToWait)
{
    (void)ticksToWait;

    assert(xQueue->itemSize <= MOCK_QUEUE_ITEM_SIZE);

    if (filled) {
        return pdFAIL;
    }

    size = xQueue->itemSize;
    memcpy(mQueueData, pvItemToQueue, xQueue->itemSize);
    filled = true;

    return pdPASS;
}

BaseType_t xQueueSendToBackFromISR(QueueHandle_t xQueue, const void* const pvItemToQueue, BaseType_t* const pxHigherPriorityTaskWoken)
{
    (void)pxHigherPriorityTaskWoken;

    assert(xQueue->itemSize <= MOCK_QUEUE_ITEM_SIZE);

    if (filled) {
        return pdFAIL;
    }

    size = xQueue->itemSize;
    memcpy(mQueueData, pvItemToQueue, xQueue->itemSize);
    filled = true;

    return pdPASS;
}

void mockSetQueueData(void* data, size_t dataSize)
{
    assert(dataSize <= MOCK_QUEUE_ITEM_SIZE);

    size = dataSize;
    filled = true;
    memcpy(mQueueData, data, dataSize);
}

bool mockGetQueueData(void* data, size_t dataSize)
{
    assert(size == dataSize);

    if (!filled) {
        return false;
    }

    memcpy(data, mQueueData, dataSize);
    return true;
}