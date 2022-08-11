/*
 * MockStm32f7xx_hal_uart.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_uart.h"
#include <string.h>
#include <assert.h>

// ------------------- Static data -------------------
static HAL_StatusTypeDef mStatusInit = HAL_OK;
static HAL_StatusTypeDef mStatusDeInit = HAL_OK;
static HAL_StatusTypeDef mStatusTransmit = HAL_OK;
static HAL_StatusTypeDef mStatusReceive = HAL_OK;
static HAL_StatusTypeDef mStatusTransmit_IT = HAL_OK;
static HAL_StatusTypeDef mStatusReceive_IT = HAL_OK;
static HAL_StatusTypeDef mStatusTransmit_DMA = HAL_OK;
static HAL_StatusTypeDef mStatusReceive_DMA = HAL_OK;
static HAL_StatusTypeDef mStatusDMAPause = HAL_OK;
static HAL_StatusTypeDef mStatusDMAResume = HAL_OK;
static HAL_StatusTypeDef mStatusDMAStop = HAL_OK;
static HAL_StatusTypeDef mStatusAbort = HAL_OK;
static HAL_StatusTypeDef mStatusAbortTransmit = HAL_OK;
static HAL_StatusTypeDef mStatusAbortReceive = HAL_OK;
static HAL_StatusTypeDef mStatusAbort_IT = HAL_OK;
static HAL_StatusTypeDef mStatusAbortTransmit_IT = HAL_OK;
static HAL_StatusTypeDef mStatusAbortReceive_IT = HAL_OK;
static HAL_StatusTypeDef mStatusReceiveToIdle_DMA = HAL_OK;

#define MOCK_UART_BUFFER_SIZE 8192 /* something large enough to put anything from the tests in */
static uint8_t uartDataBuf[MOCK_UART_BUFFER_SIZE] = { 0 };
static size_t uartDataBufLen = 0U;

// ------------------- Methods -------------------
HAL_StatusTypeDef stubHAL_UART_Init(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusInit;
}

HAL_StatusTypeDef stubHAL_UART_DeInit(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusDeInit;
}

HAL_StatusTypeDef stubHAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    (void)huart;
    (void)pData;
    (void)Size;
    (void)Timeout;

    return mStatusTransmit;
}

HAL_StatusTypeDef stubHAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    (void)huart;
    (void)pData;
    (void)Size;
    (void)Timeout;

    return mStatusReceive;
}

HAL_StatusTypeDef stubHAL_UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    (void)huart;
    (void)pData;
    (void)Size;

    return mStatusTransmit_IT;
}

HAL_StatusTypeDef stubHAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    (void)huart;
    (void)pData;
    (void)Size;

    return mStatusReceive_IT;
}

HAL_StatusTypeDef stubHAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    (void)huart;

    assert(Size <= MOCK_UART_BUFFER_SIZE);
    memcpy(uartDataBuf, pData, Size);
    uartDataBufLen = Size;

    return mStatusTransmit_DMA;
}

HAL_StatusTypeDef stubHAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    (void)huart;
    (void)pData;
    (void)Size;

    return mStatusReceive_DMA;
}

HAL_StatusTypeDef stubHAL_UART_DMAPause(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusDMAPause;
}

HAL_StatusTypeDef stubHAL_UART_DMAResume(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusDMAResume;
}

HAL_StatusTypeDef stubHAL_UART_DMAStop(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusDMAStop;
}

HAL_StatusTypeDef stubHAL_UART_Abort(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusAbort;
}

HAL_StatusTypeDef stubHAL_UART_AbortTransmit(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusAbortTransmit;
}

HAL_StatusTypeDef stubHAL_UART_AbortReceive(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusAbortReceive;
}

HAL_StatusTypeDef stubHAL_UART_Abort_IT(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusAbort_IT;
}

HAL_StatusTypeDef stubHAL_UART_AbortTransmit_IT(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusAbortTransmit_IT;
}

HAL_StatusTypeDef stubHAL_UART_AbortReceive_IT(UART_HandleTypeDef *huart)
{
    (void)huart;

    return mStatusAbortReceive_IT;
}

HAL_StatusTypeDef stubHAL_UARTEx_ReceiveToIdle_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t rxBufSize)
{
    (void)huart;
    (void)pData;
    (void)rxBufSize;

    return mStatusReceiveToIdle_DMA;
}

void mockSet_HAL_UART_All_Status(HAL_StatusTypeDef status)
{
    mStatusInit = status;
    mStatusDeInit = status;
    mStatusTransmit = status;
    mStatusReceive = status;
    mStatusTransmit_IT = status;
    mStatusReceive_IT = status;
    mStatusTransmit_DMA = status;
    mStatusReceive_DMA = status;
    mStatusDMAPause = status;
    mStatusDMAResume = status;
    mStatusDMAStop = status;
    mStatusAbort = status;
    mStatusAbortTransmit = status;
    mStatusAbortReceive = status;
    mStatusAbort_IT = status;
    mStatusAbortTransmit_IT = status;
    mStatusAbortReceive_IT = status;
    mStatusReceiveToIdle_DMA = status;
}

void mockSet_HAL_UART_Init_Status(HAL_StatusTypeDef status)
{
    mStatusInit = status;
}

void mockSet_HAL_UART_DeInit_Status(HAL_StatusTypeDef status)
{
    mStatusDeInit = status;
}

void mockSet_HAL_UART_Transmit_Status(HAL_StatusTypeDef status)
{
    mStatusTransmit = status;
}

void mockSet_HAL_UART_Receive_Status(HAL_StatusTypeDef status)
{
    mStatusReceive = status;
}

void mockSet_HAL_UART_Transmit_IT_Status(HAL_StatusTypeDef status)
{
    mStatusTransmit_IT = status;
}

void mockSet_HAL_UART_Receive_IT_Status(HAL_StatusTypeDef status)
{
    mStatusReceive_IT = status;
}

void mockSet_HAL_UART_Transmit_DMA_Status(HAL_StatusTypeDef status)
{
    mStatusTransmit_DMA = status;
}

void mockSet_HAL_UART_Receive_DMA_Status(HAL_StatusTypeDef status)
{
    mStatusReceive_DMA = status;
}

void mockSet_HAL_UART_DMAPause_Status(HAL_StatusTypeDef status)
{
    mStatusDMAPause = status;
}

void mockSet_HAL_UART_DMAResume_Status(HAL_StatusTypeDef status)
{
    mStatusDMAResume = status;
}

void mockSet_HAL_UART_DMAStop_Status(HAL_StatusTypeDef status)
{
    mStatusDMAStop = status;
}

void mockSet_HAL_UART_Abort_Status(HAL_StatusTypeDef status)
{
    mStatusAbort = status;
}

void mockSet_HAL_UART_AbortTransmit_Status(HAL_StatusTypeDef status)
{
    mStatusAbortTransmit = status;
}

void mockSet_HAL_UART_AbortReceive_Status(HAL_StatusTypeDef status)
{
    mStatusAbortReceive = status;
}

void mockSet_HAL_UART_Abort_IT_Status(HAL_StatusTypeDef status)
{
    mStatusAbort_IT = status;
}

void mockSet_HAL_UART_AbortTransmit_IT_Status(HAL_StatusTypeDef status)
{
    mStatusAbortTransmit_IT = status;
}

void mockSet_HAL_UART_AbortReceive_IT_Status(HAL_StatusTypeDef status)
{
    mStatusAbortReceive_IT = status;
}

void mockSet_HAL_UARTEx_ReceiveToIdle_DMA_Status(HAL_StatusTypeDef status)
{
    mStatusReceiveToIdle_DMA = status;
}

void mockSet_HAL_UART_Data(const void* data, const size_t dataSize)
{
    assert(dataSize <= MOCK_UART_BUFFER_SIZE);
    memcpy(uartDataBuf, data, dataSize);
    uartDataBufLen = dataSize;
}

void mockClear_HAL_UART_Data(void)
{
    uartDataBufLen = 0U;
}

size_t mockGet_HAL_UART_Len(void)
{
    return uartDataBufLen;
}

uint8_t* mockGet_HAL_UART_Data(void)
{
    return uartDataBuf;
}