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
#include <stdint.h>

#ifndef INC_FREERTOS_H
	#error "include FreeRTOS.h" must appear in source files before "include queue.h"
#endif

// ================== Define types ==================
#include "queue.h"

typedef struct {
	uint32_t count;
	uint32_t countMax;
} StaticSemaphore_t;

typedef StaticSemaphore_t* SemaphoreHandle_t;

// ================== Define methods ==================
SemaphoreHandle_t xSemaphoreCreateBinaryStatic(StaticSemaphore_t* staticSemaphore);
SemaphoreHandle_t xSemaphoreCreateMutexStatic(StaticSemaphore_t* staticSemaphore);
BaseType_t xSemaphoreTake(SemaphoreHandle_t xQueue, TickType_t xTicksToWait);
BaseType_t xSemaphoreGive(SemaphoreHandle_t xQueue);
BaseType_t xSemaphoreGiveFromISR(SemaphoreHandle_t xQueue, BaseType_t* pxHigherPriorityTaskWoken);

void mockSemaphoreSetLocked(SemaphoreHandle_t xQueue, bool locked);
void mockSemaphoreSetCount(SemaphoreHandle_t xQueue, uint32_t semphrCount);
bool mockSempahoreGetLocked(SemaphoreHandle_t xQueue);

#endif
