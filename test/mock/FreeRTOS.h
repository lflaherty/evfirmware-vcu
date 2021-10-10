/*
 * FreeRTOS.h
 * Mock for FreeRTOS FreeRTOS.h
 * 
 *  Created on: Oct 10, 2021
 *      Author: Liam Flaherty
 */

#ifndef INC_FREERTOS_H
#define INC_FREERTOS_H

#include <stdint.h>
#include <stddef.h>

// ================== Define types ==================
typedef struct
{
    // Empty for mock
} StaticTask_t;

typedef struct xSTATIC_QUEUE
{
    size_t itemSize;
    // Must be kept the same as QueueDefinition in queue.h
} StaticQueue_t;

typedef uint32_t StackType_t;
typedef uint32_t TickType_t;
typedef long BaseType_t;
typedef unsigned long UBaseType_t;

#define portTICK_PERIOD_MS 1U
#define pdFALSE			( ( BaseType_t ) 0 )
#define pdTRUE			( ( BaseType_t ) 1 )
#define pdPASS			( pdTRUE )
#define pdFAIL			( pdFALSE )

#define portYIELD_FROM_ISR( x ) ( (void)x ) /* Mock this macro to do nothing */


#endif