/*
 * i2c.h
 *
 * Driver for I2C.
 * 
 *  Created on: Jun 17, 2023
 *      Author: Liam Flaherty
 */

#ifndef COMM_I2C_I2C_H_
#define COMM_I2C_I2C_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>

#include "lib/depends/depends.h"
#include "lib/logging/logging.h"

REGISTERED_MODULE_STATIC(I2C);

typedef enum
{
  I2C_DEV0 = 0,
  I2C_DEV1,
  I2C_NUM_INTERFACES,
  I2C_DEV_INVALID
} I2C_Device_T;

typedef enum
{
  I2C_STATUS_OK                 = 0x00U,
  I2C_STATUS_NOT_READY          = 0x01U,
  I2C_STATUS_ERROR_DEPENDS      = 0x02U,
  I2C_STATUS_ERROR_INVALID_BUS  = 0x03U,
  I2C_STATUS_ERROR_HAL          = 0x04U,
  I2C_STATUS_ERROR_OS           = 0x05U,
  I2C_STATUS_ERROR_BUSY         = 0x06U,
  I2C_STATUS_ERROR_TIMEOUT      = 0x07U,
  I2C_STATUS_ERROR_DATALEN      = 0x08U,
} I2C_Status_T;

typedef struct {
  I2C_Device_T dev;
  I2C_HandleTypeDef* handle;
  IRQn_Type txIrq;
  IRQn_Type rxIrq;
} I2C_BusConfig_T;

/**
 * @brief Initialize I2C driver interface.
 * 
 * @param logger Pointer to system logger.
 * @return I2C_STATUS_OK if successful.
 */
I2C_Status_T I2C_Init(Logging_T* logger);

/**
 * @brief Configure I2C interface.
 * This should be called from main.
 * Main will retain ownership of handle pointer.
 * 
 * @param devConfig Config settings for a particular bus.
 * @return Return status. I2C_STATUS_OK for success.
 */
I2C_Status_T I2C_Config(const I2C_BusConfig_T* devConfig);

/**
 * @brief Write a number of bytes to an I2C device.
 * Operates in blocking mode, but yields to other tasks.
 * 
 * @param dev I2C device.
 * @param address Slave device address to write to.
 * @param data Data to write on I2C bus.
 * @param dataLen Length of data.
 * @return I2C_STATUS_OK if successful. 
 */
I2C_Status_T I2C_Write(
    const I2C_Device_T dev,
    uint16_t address,
    uint8_t* data,
    uint16_t dataLen);

/**
 * @brief Read a number of bytes from an I2C device.
 * Operates in blocking mode, but yields to other tasks.
 * 
 * @param dev 
 * @param address Slave device address to read from.
 * @param data 
 * @param dataLen 
 * @return I2C_Status_T 
 */
I2C_Status_T I2C_Read(
    const I2C_Device_T dev,
    uint16_t address,
    uint8_t* data,
    uint16_t dataLen);

I2C_Status_T I2C_WriteReg(
    const I2C_Device_T dev,
    uint16_t devAddress,
    uint16_t regAddress,
    uint16_t regSize,
    uint8_t* data,
    uint16_t dataLen);

I2C_Status_T I2C_ReadReg(
    const I2C_Device_T dev,
    uint16_t devAddress,
    uint16_t regAddress,
    uint16_t regSize,
    uint8_t* data,
    uint16_t dataLen);

#endif // COMM_I2C_I2C_H_
