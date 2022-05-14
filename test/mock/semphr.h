/**
 * semphr.h
 * Mock for FreeRTOS semphr.h
 * 
 *  Created on: Oct 17 2021
 *      Author: Liam Flaherty
 */

#ifndef SEMPHR_H
#define SEMPHR_H

#include <stddef.h>

#ifndef INC_FREERTOS_H
	#error "include FreeRTOS.h" must appear in source files before "include queue.h"
#endif

// ================== Define types ==================
#include "queue.h"

typedef QueueHandle_t SemaphoreHandle_t; // this is how FreeRTOS defines it, so just reuse that here...

// ================== Define methods ==================
SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t* staticSemaphore);
BaseType_t xSemaphoreTake(SemaphoreHandle_t xQueue, TickType_t xTicksToWait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t xQueue);

void mockSemaphoreSetLocked(bool locked);
bool mockSempahoreGetLocked(void);

#endif