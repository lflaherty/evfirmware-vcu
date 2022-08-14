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

#define pdMS_TO_TICKS(x) (x * 10)  /* Good enough for the mock */

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

#define MOCK_STREAMBUFFER_SIZE 8192 /* something large enough to put anything from the tests in */
typedef struct
{
    size_t itemSize;

    // mock buffer data for tests:
    uint8_t streamBufferData[MOCK_STREAMBUFFER_SIZE];
    size_t start;
    size_t end;
    // Must be kept the same as StreamBufferDef_t in stream_buffer.h
} StaticStreamBuffer_t;

typedef StaticQueue_t StaticSemaphore_t;

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