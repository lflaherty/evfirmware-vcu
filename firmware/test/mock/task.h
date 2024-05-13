/*
 * task.h
 * Mock for FreeRTOS task.h
 * 
 *  Created on: Oct 10, 2021
 *      Author: Liam Flaherty
 */

#ifndef INC_TASK_H
#define INC_TASK_H

#ifndef INC_FREERTOS_H
	#error "include FreeRTOS.h must appear in source files before include task.h"
#endif

// ================== Define types ==================
struct tskTaskControlBlock;
typedef struct tskTaskControlBlock* TaskHandle_t;

typedef void (*TaskFunction_t)( void * );

// ================== Define methods ==================
TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                                const char* const pcName,
                                const uint32_t ulStackDepth,
                                void* const pvParameters,
                                UBaseType_t uxPriority,
                                StackType_t* const puxStackBuffer,
                                StaticTask_t* const pxTaskBuffer);
void vTaskNotifyGiveFromISR(TaskHandle_t xTaskToNotify, BaseType_t* pxHigherPriorityTaskWoken);
uint32_t ulTaskNotifyTake(BaseType_t xClearCountOnExit, TickType_t xTicksToWait);

void mockSetTaskNotifyValue(uint32_t value);
uint32_t mockGetTaskNotifyValue(void);

#endif
