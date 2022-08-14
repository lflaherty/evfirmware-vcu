/*
 * mockStreamBuffer.c
 * Mock implementation for FreeRTOS stream_buffer.h
 * 
 *  Created on: Oct 10, 2021
 *      Author: Liam Flaherty
 */

#include "FreeRTOS.h"
#include "stream_buffer.h"

#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

// ------------------- Static data -------------------


// ------------------- Methods -------------------
StreamBufferHandle_t xStreamBufferCreateStatic(
        size_t xBufferSizeBytes,
        size_t xTriggerLevelBytes,
        uint8_t *pucStreamBufferStorageArea,
        StaticStreamBuffer_t *pxStaticStreamBuffer)
{
    (void)xBufferSizeBytes;
    (void)xTriggerLevelBytes;
    (void)pucStreamBufferStorageArea;

    StreamBufferHandle_t handle = (StreamBufferHandle_t)pxStaticStreamBuffer;

    return handle;
}

size_t xStreamBufferSend(
        StreamBufferHandle_t xStreamBuffer,
        const void *pvTxData,
        size_t xDataLengthBytes,
        TickType_t xTicksToWait)
{
    (void)xStreamBuffer;
    (void)xTicksToWait;

    size_t mockBufferAvailBytes = MOCK_STREAMBUFFER_SIZE - xStreamBuffer->end;
    assert(mockBufferAvailBytes >= xDataLengthBytes);

    memcpy(xStreamBuffer->streamBufferData + xStreamBuffer->end, pvTxData, xDataLengthBytes);
    xStreamBuffer->end += xDataLengthBytes;

    return xDataLengthBytes;
}

size_t xStreamBufferSendFromISR(
        StreamBufferHandle_t xStreamBuffer,
        const void *pvTxData,
        size_t xDataLengthBytes,
        BaseType_t *pxHigherPriorityTaskWoken)
{
    *pxHigherPriorityTaskWoken = pdFALSE;
    return xStreamBufferSend(xStreamBuffer, pvTxData, xDataLengthBytes, 0U);
}

size_t xStreamBufferReceive(
        StreamBufferHandle_t xStreamBuffer,
        void* pvRxData,
        size_t xBufferLengthBytes,
        TickType_t xTicksToWait)
{
    (void)xStreamBuffer;
    (void)xTicksToWait;

    size_t contentsSize = xStreamBuffer->end - xStreamBuffer->start;
    size_t numCopyBytes = (contentsSize > xBufferLengthBytes) ?
                           xBufferLengthBytes : contentsSize;

    memcpy(pvRxData, xStreamBuffer->streamBufferData + xStreamBuffer->start, numCopyBytes);

    xStreamBuffer->start += numCopyBytes;
    return numCopyBytes;
}

size_t xStreamBufferReceiveFromISR(
        StreamBufferHandle_t xStreamBuffer,
        void* pvRxData,
        size_t xBufferLengthBytes,
        BaseType_t* pxHigherPriorityTaskWoken)
{
    *pxHigherPriorityTaskWoken = pdFALSE;
    return xStreamBufferReceive(xStreamBuffer, pvRxData, xBufferLengthBytes, 0U);
}

BaseType_t xStreamBufferIsEmpty(StreamBufferHandle_t xStreamBuffer)
{
    (void)xStreamBuffer;

    size_t size = xStreamBuffer->end - xStreamBuffer->start;
    return size > 0 ? pdFALSE : pdTRUE;
}

void mockSetStreamBufferData(
        StreamBufferHandle_t xStreamBuffer,
        const void* data,
        const size_t dataSize)
{
    assert(dataSize <= MOCK_STREAMBUFFER_SIZE);

    xStreamBuffer->start = 0;
    xStreamBuffer->end = dataSize;
    memcpy(xStreamBuffer->streamBufferData, data, dataSize);
}

void mockClearStreamBufferData(StreamBufferHandle_t xStreamBuffer)
{
    xStreamBuffer->start = 0;
    xStreamBuffer->end = 0;
}

size_t mockGetStreamBufferLen(StreamBufferHandle_t xStreamBuffer)
{
    size_t size = xStreamBuffer->end - xStreamBuffer->start;
    return size;
}

bool mockGetStreamBufferData(
        StreamBufferHandle_t xStreamBuffer,
        void* data,
        const size_t maxSize)
{
    size_t size = xStreamBuffer->end - xStreamBuffer->start;
    assert(maxSize >= size);

    memcpy(data, xStreamBuffer->streamBufferData + xStreamBuffer->start, size);
    return true;
}