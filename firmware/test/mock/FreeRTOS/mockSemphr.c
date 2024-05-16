/**
 * mockSemphr.c
 * Mock implementation for FreeRTOS semphr.h
 * 
 *  Created on: Oct 17 2021
 *      Author: Liam Flaherty
 */

#include "FreeRTOS.h"
#include "semphr.h"

#include <string.h>
#include <stdbool.h>
#include <assert.h>

// ------------------- Static data -------------------

// ------------------- Methods -------------------
SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t* staticSemaphore)
{
    assert(staticSemaphore != NULL);

    SemaphoreHandle_t handle = (SemaphoreHandle_t)staticSemaphore;
    handle->count = 0;
    handle->countMax = 1;
    return handle;
}

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t* staticSemaphore)
{
    assert(staticSemaphore != NULL);

    SemaphoreHandle_t handle = (SemaphoreHandle_t)staticSemaphore;
    handle->count = 1;
    handle->countMax = 1;
    return handle;
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t xQueue, TickType_t xTicksToWait)
{
    (void)xTicksToWait;

    assert(xQueue != NULL);

    if (xQueue->count == 0) {
        return pdFALSE;
    } else {
        xQueue->count--;
        return pdTRUE;
    }
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t xQueue)
{
    assert(xQueue != NULL);

    if (xQueue->count == xQueue->countMax) {
        return pdFALSE;
    } else {
        xQueue->count++;
        return pdTRUE;
    }
}

BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t xQueue, BaseType_t* pxHigherPriorityTaskWoken)
{
    if (pxHigherPriorityTaskWoken != NULL) {
        *pxHigherPriorityTaskWoken = pdFALSE;
    }
    return xSemaphoreGive(xQueue);
}

void mockSemaphoreSetLocked(SemaphoreHandle_t xQueue, bool locked)
{
    assert(xQueue != NULL);
    // This mock only works for mutexes and binary semaphores
    assert(xQueue->countMax == 1);
    xQueue->count = locked ? 0 : 1;
}

void mockSemaphoreSetCount(SemaphoreHandle_t xQueue, uint32_t semphrCount)
{
    assert(xQueue != NULL);
    // This mock only works for mutexes and binary semaphores
    assert(semphrCount <= xQueue->countMax);
    xQueue->count = semphrCount;
}

bool mockSempahoreGetLocked(SemaphoreHandle_t xQueue)
{
    assert(xQueue != NULL);
    // This mock only works for mutexes and binary semaphores
    assert(xQueue->countMax == 1);
    return xQueue->count > 0 ? false : true;
}
