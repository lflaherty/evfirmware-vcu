/*
 * queue.h
 * Mock for FreeRTOS queue.h
 * 
 *  Created on: Oct 10, 2021
 *      Author: Liam Flaherty
 */

#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

#ifndef INC_FREERTOS_H
	#error "include FreeRTOS.h" must appear in source files before "include queue.h"
#endif

// ================== Define types ==================
struct QueueDefinition
{
    size_t itemSize;
    // must be kept the same as StaticQueue_t in FreeRTOS.h
};
typedef struct QueueDefinition* QueueHandle_t;

// ================== Define methods ==================
QueueHandle_t xQueueCreateStatic(const UBaseType_t uxQueueLength, const UBaseType_t uxItemSize, uint8_t* pucQueueStorage, StaticQueue_t* pxStaticQueue);

BaseType_t xQueueReceive(QueueHandle_t xQueue, void* const pvBuffer, TickType_t xTicksToWait);

BaseType_t xQueueSendToBackFromISR(QueueHandle_t xQueue, const void* const pvItemToQueue, BaseType_t* const pxHigherPriorityTaskWoken);

/**
 * @brief Sets the queue to return this data upon next xQueueReceive
 * @param data Pointer to data to use
 * @param dataSize size of data. Note that this will be used to copy the data.
 */
void mockSetQueueDataPtr(void* data);

#endif