/*
 * i2c.c
 *
 * Driver for I2C.
 * 
 *  Created on: Jun 17, 2023
 *      Author: Liam Flaherty
 */

#include "i2c.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "semphr.h"

REGISTERED_MODULE_STATIC_DEF(I2C);

// ------------------- Private data -------------------
static Logging_T* mLog;

#define RTOS_MS_WAIT (5)
#define RTOS_TICKS_WAIT pdMS_TO_TICKS(RTOS_MS_WAIT)

#define DMA_BUF_LEN 2048

/**
 * @brief I2C interface storage
 */
struct I2C_Instance {
  bool inUse;
  I2C_HandleTypeDef* handle;
  IRQn_Type txIrq;
  IRQn_Type rxIrq;

  // DMA buffers
  uint8_t dmaBuf[DMA_BUF_LEN];
  uint16_t dmaBufLen;

  // ISR to thread synchronization
  SemaphoreHandle_t busyMutex; // main thread access control
  StaticSemaphore_t busyMutexBuffer;
  SemaphoreHandle_t xferInProgressSem; // ISR -> thread synchronization
  StaticSemaphore_t xferInProgressSemBuffer;
};

static struct I2C_Instance i2cInstances[I2C_NUM_INTERFACES];

// ------------------- Private methods -------------------
static I2C_Device_T handleToDevice(const I2C_HandleTypeDef *hi2c)
{
  for (uint16_t i = 0; i < I2C_NUM_INTERFACES; ++i) {
    if (i2cInstances[i].handle->Instance == hi2c->Instance &&
        i2cInstances[i].inUse) {
      return (I2C_Device_T)i;
    }
  }

  return I2C_DEV_INVALID;
}

/**
 * @brief Common ISR handler to just signal completion of an I2C transfer.
 * 
 * @param hi2c 
 */
static void I2C_XferCplt_ISR(I2C_HandleTypeDef* hi2c)
{
  I2C_Device_T deviceId = handleToDevice(hi2c);
  if (deviceId >= I2C_NUM_INTERFACES) {
    return;
  }

  // Just signal to thread that the xfer has finished
  struct I2C_Instance* i2cDev = &i2cInstances[deviceId];
  xSemaphoreGiveFromISR(i2cDev->xferInProgressSem, NULL);
}

/**
 * @brief ISR handler
 * 
 * @param hi2c 
 */
void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef* hi2c)
{
  I2C_XferCplt_ISR(hi2c);
}

/**
 * @brief ISR handler
 * 
 * @param hi2c 
 */
void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef* hi2c)
{
  I2C_XferCplt_ISR(hi2c);
}

// ------------------- Public methods -------------------
I2C_Status_T I2C_Init(Logging_T* logger)
{
  mLog = logger;
  Log_Print(mLog, "I2C_Init begin\n");
  DEPEND_ON(logger, I2C_STATUS_ERROR_DEPENDS);

  // Reset memory
  memset(i2cInstances, 0, sizeof(i2cInstances));

  REGISTER_STATIC(I2C, I2C_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "I2C_Init complete\n");
  return I2C_STATUS_OK;
}

I2C_Status_T I2C_Config(const I2C_BusConfig_T* devConfig)
{
  Log_Print(mLog, "I2C_Config begin ");
  DEPEND_ON_STATIC(I2C, I2C_STATUS_ERROR_DEPENDS);

  switch (devConfig->dev) {
    case I2C_DEV0:
      Log_Print(mLog, "I2C0\n");
      break;
    case I2C_DEV1:
      Log_Print(mLog, "I2C1\n");
      break;
    default:
      return I2C_STATUS_ERROR_INVALID_BUS;
  }

  struct I2C_Instance* i2cDev = &i2cInstances[devConfig->dev];
  if (i2cDev->inUse) {
    Log_Print(mLog, "I2C_Config Error: Instance already in use\n");
    return I2C_STATUS_ERROR_INVALID_BUS;
  }

  // Initialize device specific data
  memset(i2cDev, 0, sizeof(struct I2C_Instance));
  i2cDev->inUse = true;
  i2cDev->handle = devConfig->handle;
  i2cDev->txIrq = devConfig->txIrq;
  i2cDev->rxIrq = devConfig->rxIrq;

  i2cDev->busyMutex = xSemaphoreCreateMutexStatic(&i2cDev->busyMutexBuffer);
  if (NULL == i2cDev->busyMutex) {
    // sempahore didn't initalize properly
    return I2C_STATUS_ERROR_OS;
  }
  i2cDev->xferInProgressSem = xSemaphoreCreateMutexStatic(&i2cDev->xferInProgressSemBuffer);
  if (NULL == i2cDev->xferInProgressSem) {
    // sempahore didn't initalize properly
    return I2C_STATUS_ERROR_OS;
  }

  Log_Print(mLog, "I2C_Config complete\n");
  return I2C_STATUS_OK;
}

I2C_Status_T I2C_Write(
    const I2C_Device_T dev,
    uint16_t address,
    uint8_t* data,
    uint16_t dataLen)
{
  if (dev >= I2C_NUM_INTERFACES) {
    return I2C_STATUS_ERROR_INVALID_BUS;
  }
  struct I2C_Instance* i2cDev = &i2cInstances[dev];

  // Take device or wait for any previous operations to complete
  if (pdTRUE != xSemaphoreTake(i2cDev->busyMutex, RTOS_TICKS_WAIT)) {
    // couldn't obtain semaphore
    return I2C_STATUS_ERROR_BUSY;
  }

  if (dataLen >= DMA_BUF_LEN) {
    xSemaphoreGive(i2cDev->busyMutex);
    return I2C_STATUS_ERROR_DATALEN;
  }
  memcpy(i2cDev->dmaBuf, data, dataLen);
  i2cDev->dmaBufLen = dataLen;

  // Start DMA transfer
  uint16_t writeAddr = address;
  writeAddr &= ~0x01; // Set R/W bit to write (0)
  HAL_StatusTypeDef status = HAL_I2C_Master_Transmit_DMA(
      i2cDev->handle,
      writeAddr,
      i2cDev->dmaBuf,
      i2cDev->dmaBufLen);
  if (HAL_OK != status) {
    xSemaphoreGive(i2cDev->busyMutex);
    return I2C_STATUS_ERROR_HAL;
  }

  // Wait for transfer to complete
  if (pdTRUE != xSemaphoreTake(i2cDev->xferInProgressSem, RTOS_TICKS_WAIT)) {
    // TODO what is the ISR doing in this state?
    // TODO abort
    xSemaphoreGive(i2cDev->busyMutex);
    return I2C_STATUS_ERROR_TIMEOUT;
  }

  // Done with device
  xSemaphoreGive(i2cDev->busyMutex);

  return I2C_STATUS_OK;
}

I2C_Status_T I2C_Read(
    const I2C_Device_T dev,
    uint16_t address,
    uint8_t* data,
    uint16_t dataLen)
{
  if (dev >= I2C_NUM_INTERFACES) {
    return I2C_STATUS_ERROR_INVALID_BUS;
  }
  struct I2C_Instance* i2cDev = &i2cInstances[dev];

  // Take device or wait for any previous operations to complete
  if (pdTRUE != xSemaphoreTake(i2cDev->busyMutex, RTOS_TICKS_WAIT)) {
    // couldn't obtain semaphore
    return I2C_STATUS_ERROR_BUSY;
  }

  // setup DMA length
  if (dataLen >= DMA_BUF_LEN) {
    xSemaphoreGive(i2cDev->busyMutex);
    return I2C_STATUS_ERROR_DATALEN;
  }
  i2cDev->dmaBufLen = dataLen;

  // Start DMA transfer
  uint16_t readAddr = address;
  readAddr |= 0x01; // Set R/W bit to read (1)
  HAL_StatusTypeDef status = HAL_I2C_Master_Receive_DMA(
      i2cDev->handle,
      readAddr,
      i2cDev->dmaBuf,
      i2cDev->dmaBufLen);
  if (HAL_OK != status) {
    xSemaphoreGive(i2cDev->busyMutex);
    return I2C_STATUS_ERROR_HAL;
  }

  // Wait for transfer to complete
  if (pdTRUE != xSemaphoreTake(i2cDev->xferInProgressSem, RTOS_TICKS_WAIT)) {
    // TODO what is the ISR doing in this state?
    // TODO abort
    xSemaphoreGive(i2cDev->busyMutex);
    return I2C_STATUS_ERROR_TIMEOUT;
  }

  // copy back DMA buffer
  memcpy(data, i2cDev->dmaBuf, i2cDev->dmaBufLen);

  // Done with device
  xSemaphoreGive(i2cDev->busyMutex);

  return I2C_STATUS_OK;
}

I2C_Status_T I2C_WriteReg(
    const I2C_Device_T dev,
    uint16_t devAddress,
    uint16_t regAddress,
    uint16_t regSize,
    uint8_t* data,
    uint16_t dataLen)
{
  (void)dev;
  (void)devAddress;
  (void)regAddress;
  (void)regSize;
  (void)data;
  (void)dataLen;
  // not implemented
  return I2C_STATUS_NOT_READY;
}

I2C_Status_T I2C_ReadReg(
    const I2C_Device_T dev,
    uint16_t devAddress,
    uint16_t regAddress,
    uint16_t regSize,
    uint8_t* data,
    uint16_t dataLen)
{
  (void)dev;
  (void)devAddress;
  (void)regAddress;
  (void)regSize;
  (void)data;
  (void)dataLen;
  // not implemented
  return I2C_STATUS_NOT_READY;
}
