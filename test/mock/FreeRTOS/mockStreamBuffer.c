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
#define MOCK_STREAMBUFFER_SIZE 8192 /* something large enough to put anything from the tests in */
static uint8_t mStreamBufferData[MOCK_STREAMBUFFER_SIZE]; // single data entry in stream_buffer
static size_t start = 0;
static size_t end = 0;

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

size_t xStreamBufferSend( StreamBufferHandle_t xStreamBuffer,
                          const void *pvTxData,
                          size_t xDataLengthBytes,
                          TickType_t xTicksToWait)
{
    (void)xStreamBuffer;
    (void)xTicksToWait;

    size_t mockBufferAvailBytes = MOCK_STREAMBUFFER_SIZE - end;
    assert(mockBufferAvailBytes >= xDataLengthBytes);

    memcpy(mStreamBufferData + end, pvTxData, xDataLengthBytes);
    end += xDataLengthBytes;

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

    size_t contentsSize = end - start;
    size_t numCopyBytes = (contentsSize > xBufferLengthBytes) ?
                           xBufferLengthBytes : contentsSize;

    memcpy(pvRxData, mStreamBufferData, numCopyBytes);
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

    size_t size = end - start;
    return size > 0 ? pdFALSE : pdTRUE;
}

void mockSetStreamBufferData(const void* data, const size_t dataSize)
{
    assert(dataSize <= MOCK_STREAMBUFFER_SIZE);

    start = 0;
    end = dataSize;
    memcpy(mStreamBufferData, data, dataSize);
}

void mockClearStreamBufferData(void)
{
    start = 0;
    end = 0;
}

size_t mockGetStreamBufferLen(void)
{
    size_t size = end - start;
    return size;
}

bool mockGetStreamBufferData(void* data, const size_t maxSize)
{
    size_t size = end - start;
    assert(maxSize >= size);

    memcpy(data, mStreamBufferData + start, size);
    return true;
}