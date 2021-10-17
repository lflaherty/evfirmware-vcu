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

// ------------------- Static data -------------------

// ------------------- Methods -------------------
SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t* staticSemaphore)
{
    (void)staticSemaphore;

    SemaphoreHandle_t handle = (SemaphoreHandle_t)staticSemaphore;
    return handle;
}

BaseType_t xSemaphoreTake(SemaphoreHandle_t xQueue, TickType_t xTicksToWait)
{
    (void)xQueue;
    (void)xTicksToWait;
    return pdTRUE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t xQueue)
{
    (void)xQueue;
    return pdTRUE;
}