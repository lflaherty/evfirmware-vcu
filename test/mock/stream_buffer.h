/*
 * stream_buffer.h
 * Mock for FreeRTOS stream_buffer.h
 * 
 *  Created on: Aug 2 2022
 *      Author: Liam Flaherty
 */

#ifndef STREAM_BUFFER_H
#define STREAM_BUFFER_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

#ifndef INC_FREERTOS_H
	#error "include FreeRTOS.h" must appear in source files before "include stream_buffer.h"
#endif

// ================== Define types ==================
struct StreamBufferDef_t
{
    size_t itemSize;
    // must be kept the same as StaticStreamBuffer_t in FreeRTOS.h
};
typedef struct StreamBufferDef_t* StreamBufferHandle_t;

// ================== Define methods ==================
StreamBufferHandle_t xStreamBufferCreateStatic(
    size_t xBufferSizeBytes,
    size_t xTriggerLevelBytes,
    uint8_t* pucStreamBufferStorageArea,
    StaticStreamBuffer_t* pxStaticStreamBuffer);

size_t xStreamBufferSend(
    StreamBufferHandle_t xStreamBuffer,
    const void* pvTxData,
    size_t xDataLengthBytes,
    TickType_t xTicksToWait);

size_t xStreamBufferSendFromISR(
    StreamBufferHandle_t xStreamBuffer,
    const void* pvTxData,
    size_t xDataLengthBytes,
    BaseType_t* pxHigherPriorityTaskWoken);

size_t xStreamBufferReceive(
    StreamBufferHandle_t xStreamBuffer,
    void* pvRxData,
    size_t xBufferLengthBytes,
    TickType_t xTicksToWait);

size_t xStreamBufferReceiveFromISR(
    StreamBufferHandle_t xStreamBuffer,
    void* pvRxData,
    size_t xBufferLengthBytes,
    BaseType_t* pxHigherPriorityTaskWoken);

BaseType_t xStreamBufferIsEmpty(StreamBufferHandle_t xStreamBuffer);

/**
 * @brief Sets the stream_buffer to return this data upon next xQueueReceive
 * @param data Pointer to data to use
 * @param dataSize size of data. Note that this will be used to copy the data.
 */
void mockSetStreamBufferData(const void* data, const size_t dataSize);

/**
 * @brief Sets the contents of the stream buffer mock to empty.
 * 
 */
void mockClearStreamBufferData(void);

/**
 * @brief Gets the number of elements sent to stream buffer.
 * 
 * @return size_t Number of items in stream buffer.
 */
size_t mockGetStreamBufferLen(void);

/**
 * @brief Gets the data item currently in stream_buffer
 * @param data Pointer to location to copy data to
 * @param maxSize Size of copy out buffer.
 * @returns True if data is present.
 */
bool mockGetStreamBufferData(void* data, const size_t maxSize);

#endif