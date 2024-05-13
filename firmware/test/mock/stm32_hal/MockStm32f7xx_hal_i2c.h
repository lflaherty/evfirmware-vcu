/*
 * MockStm32f7xx_hal_i2c.h
 * Some excerpts from stm32f7xx_hal_i2c.h in STM32 HAL.
 *
 *  Created on: 23 Jul 2023
 *      Author: Liam Flaherty
 */

#ifndef MOCK_STM32F7xx_HAL_I2C_H_
#define MOCK_STM32F7xx_HAL_I2C_H_

#include <stdint.h>
#include <stddef.h>
#include "MockStm32f7xx_hal_def.h"

// ================== Define types ==================
typedef struct
{
    // Empty for mock
    uint8_t tmp;
} I2C_InitTypeDef;

typedef struct
{
    // Empty for mock
    uint8_t tmp;
} I2C_TypeDef;


typedef struct 
{
  I2C_TypeDef* Instance;
} I2C_HandleTypeDef;

// Interrupts
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode);
void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c);
void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c);

// ================== Define methods ==================
HAL_StatusTypeDef stubHAL_I2C_Master_Transmit_DMA(
    I2C_HandleTypeDef *hi2c,
    uint16_t DevAddress,
    uint8_t *pData,
    uint16_t Size);

HAL_StatusTypeDef stubHAL_I2C_Master_Receive_DMA(
    I2C_HandleTypeDef *hi2c,
    uint16_t DevAddress,
    uint8_t *pData,
    uint16_t Size);

HAL_StatusTypeDef stubHAL_I2C_Mem_Write_DMA(
    I2C_HandleTypeDef *hi2c,
    uint16_t DevAddress,
    uint16_t MemAddress,
    uint16_t MemAddSize,
    uint8_t *pData,
    uint16_t Size);

HAL_StatusTypeDef stubHAL_I2C_Mem_Read_DMA(
    I2C_HandleTypeDef *hi2c,
    uint16_t DevAddress,
    uint16_t MemAddress,
    uint16_t MemAddSize,
    uint8_t *pData,
    uint16_t Size);

// Replace real methods with mock stubs
#define HAL_I2C_Master_Transmit_DMA stubHAL_I2C_Master_Transmit_DMA
#define HAL_I2C_Master_Receive_DMA stubHAL_I2C_Master_Receive_DMA
#define HAL_I2C_Mem_Write_DMA stubHAL_I2C_Mem_Write_DMA
#define HAL_I2C_Mem_Read_DMA stubHAL_I2C_Mem_Read_DMA

// ================== Mock control methods ==================
void mockSet_HAL_I2C_AllStatus(HAL_StatusTypeDef status);
void mockSet_HAL_I2C_Master_Transmit_DMA_Status(HAL_StatusTypeDef status);
void mockSet_HAL_I2C_Master_Receive_DMA_Status(HAL_StatusTypeDef status);
void mockSet_HAL_I2C_Mem_Write_DMA_Status(HAL_StatusTypeDef status);
void mockSet_HAL_I2C_Mem_Read_DMA_Status(HAL_StatusTypeDef status);

void mockSet_HAL_I2C_DataBuf(uint8_t* data, uint16_t dataLen);
uint8_t* mockGet_HAL_I2C_DataBuf(void);
uint16_t mockGet_HAL_I2C_DataBufLen(void);
uint16_t mockGet_HAL_I2C_DevAddr(void);


#endif // MOCK_STM32F7xx_HAL_I2C_H_
