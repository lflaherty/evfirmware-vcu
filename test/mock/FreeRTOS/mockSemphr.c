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

// ------------------- Static data -------------------
static bool mLocked = false;

// ------------------- Methods -------------------
SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t* staticSemaphore)
{
    (void)staticSemaphore;

    SemaphoreHandle_t handle = (SemaphoreHandle_t)staticSemaphore;
    return handle;
}

SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t* staticSemaphore)
{
    (void)staticSemaphore;

    SemaphoreHandle_t handle = (SemaphoreHandle_t)staticSemaphore;
    return handle;
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t xQueue, TickType_t xTicksToWait)
{
    (void)xQueue;
    (void)xTicksToWait;
    if (mLocked) {
        return pdFALSE;
    } else {
        mLocked = true;
        return pdTRUE;
    }
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t xQueue)
{
    (void)xQueue;
    if (mLocked) {
        mLocked = false;
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}

void mockSemaphoreSetLocked(bool locked)
{
    mLocked = locked;
}

bool mockSempahoreGetLocked(void)
{
    return mLocked;
}