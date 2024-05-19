/*
 * MockStm32f7xx_hal_i2c.c
 *
 *  Created on: 23 Jul 2023
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_i2c.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// ------------------- Static data -------------------
static HAL_StatusTypeDef mStatus_Master_Transmit_DMA = HAL_OK;
static HAL_StatusTypeDef mStatus_Master_Receive_DMA = HAL_OK;
static HAL_StatusTypeDef mStatus_Mem_Write_DMA = HAL_OK;
static HAL_StatusTypeDef mStatus_Mem_Read_DMA = HAL_OK;

#define MOCK_DATABUF_LEN 4096
uint16_t devAddr = 0;
uint8_t dataBuf[MOCK_DATABUF_LEN] = { 0 };
uint16_t dataBufLen = 0;

// ------------------- Helpers -------------------

// ------------------- Methods -------------------
HAL_StatusTypeDef stubHAL_I2C_Master_Transmit_DMA(
    I2C_HandleTypeDef *hi2c,
    uint16_t DevAddress,
    uint8_t *pData,
    uint16_t Size)
{
    (void)hi2c;
    assert(Size <= MOCK_DATABUF_LEN);

    devAddr = DevAddress;
    dataBufLen = Size;
    memcpy(dataBuf, pData, Size);

    return mStatus_Master_Transmit_DMA;
}

HAL_StatusTypeDef stubHAL_I2C_Master_Receive_DMA(
    I2C_HandleTypeDef *hi2c,
    uint16_t DevAddress,
    uint8_t *pData,
    uint16_t Size)
{
    (void)hi2c;
    assert(Size <= MOCK_DATABUF_LEN);

    assert(Size == dataBufLen);

    devAddr = DevAddress;
    memcpy(pData, dataBuf, Size);

    return mStatus_Master_Transmit_DMA;
}


HAL_StatusTypeDef stubHAL_I2C_Mem_Write_DMA(
    I2C_HandleTypeDef *hi2c,
    uint16_t DevAddress,
    uint16_t MemAddress,
    uint16_t MemAddSize,
    uint8_t *pData,
    uint16_t Size)
{
    uint16_t txArrayLen = MemAddSize + Size;
    uint8_t txArray[txArrayLen];

    // Fill in memory address in the correct order
    int shift = 8 * (MemAddSize - 1);
    for (uint16_t i = 0; i < MemAddSize; ++i) {
        uint8_t addrByte = (uint8_t)((MemAddress >> shift) & 0xFF);
        shift -= 8;
        txArray[i] = addrByte;
    }

    memcpy(txArray + MemAddSize, pData, Size);

    return stubHAL_I2C_Master_Transmit_DMA(
        hi2c,
        DevAddress,
        txArray,
        txArrayLen);
}

HAL_StatusTypeDef stubHAL_I2C_Mem_Read_DMA(
    I2C_HandleTypeDef *hi2c,
    uint16_t DevAddress,
    uint16_t MemAddress,
    uint16_t MemAddSize,
    uint8_t *pData,
    uint16_t Size)
{
    // Select the address via a dedicated I2C call
    uint8_t addressCmd[MemAddSize];
    // Fill in memory address in the correct order
    int shift = 8 * (MemAddSize - 1);
    for (uint16_t i = 0; i < MemAddSize; ++i) {
        uint8_t addrByte = (uint8_t)((MemAddress >> shift) & 0xFF);
        shift -= 8;
        addressCmd[i] = addrByte;
    }

    HAL_StatusTypeDef status = stubHAL_I2C_Master_Transmit_DMA(
        hi2c,
        DevAddress,
        addressCmd,
        MemAddSize);
    if (HAL_OK != status) {
        return status;
    }

    // now issue actual read instruction
    return stubHAL_I2C_Master_Receive_DMA(
        hi2c,
        DevAddress,
        pData,
        Size);
}

// ------------------- Mock control methods -------------------
void mockSet_HAL_I2C_AllStatus(HAL_StatusTypeDef status)
{
    mStatus_Master_Transmit_DMA = status;
    mStatus_Master_Receive_DMA = status;
    mStatus_Mem_Write_DMA = status;
    mStatus_Mem_Read_DMA = status;
}

void mockSet_HAL_I2C_Master_Transmit_DMA_Status(HAL_StatusTypeDef status)
{
    mStatus_Master_Transmit_DMA = status;
}

void mockSet_HAL_I2C_Master_Receive_DMA_Status(HAL_StatusTypeDef status)
{
    mStatus_Master_Receive_DMA = status;
}

void mockSet_HAL_I2C_Mem_Write_DMA_Status(HAL_StatusTypeDef status)
{
    mStatus_Mem_Write_DMA = status;
}

void mockSet_HAL_I2C_Mem_Read_DMA_Status(HAL_StatusTypeDef status)
{
    mStatus_Mem_Read_DMA = status;
}

void mockSet_HAL_I2C_DataBuf(uint8_t* data, uint16_t dataLen)
{
    assert(dataLen <= MOCK_DATABUF_LEN);
    memcpy(dataBuf, data, dataLen);
    dataBufLen = dataLen;
}

uint8_t* mockGet_HAL_I2C_DataBuf(void)
{
    return dataBuf;
}

uint16_t mockGet_HAL_I2C_DataBufLen(void)
{
    return dataBufLen;
}

uint16_t mockGet_HAL_I2C_DevAddr(void)
{
    return devAddr;
}
