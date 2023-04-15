/*
 * uart.c
 *
 *  Created on: May 23, 2021
 *      Author: Liam Flaherty
 */

#include "uart.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

REGISTERED_MODULE_STATIC_DEF(UART);

// ------------------- Private data -------------------
static Logging_T* mLog;

// uart operating info
struct uartInfo {
  volatile bool isEnabled;
  UART_HandleTypeDef* handle;

  // output stream buffer
  bool outputSbEnabled;
  StreamBufferHandle_t outputSb;

  // DMA buffers
  uint8_t uartDmaRx[UART_MAX_DMA_LEN];
  uint8_t uartDmaTx[UART_MAX_DMA_LEN];

  // Interrupts
  IRQn_Type rxIrq;

  // tx info
  volatile bool txInProgress;
  uint8_t txPendingStorage[UART_MAX_DMA_LEN];
  StaticStreamBuffer_t txPendingStreamStruct;
  StreamBufferHandle_t txPendingStreamHandle;
};

static struct uartInfo interfaces[UART_NUM_INTERFACES];

static UART_Device_T uartHandleToDevice(const UART_HandleTypeDef* huart)
{
  if (USART1 == huart->Instance) {
    return UART_DEV1;
  } else if (USART2 == huart->Instance) {
    return UART_DEV2;
  } else if (USART3 == huart->Instance) {
    return UART_DEV3;
  } else if (UART4 == huart->Instance) {
    return UART_DEV4;
  } else if (UART5 == huart->Instance) {
    return UART_DEV5;
  } else if (USART6 == huart->Instance) {
	  return UART_DEV6;
  } else {
    // uart instance not implemented
    return UART_DEV_INVALID;
  }
}

/**
 * @brief UART DMA Rx interrupt
 *
 * @brief huart UART handle provided by interrupt
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size)
{
  UART_Device_T uartDev = uartHandleToDevice(huart);
  if (UART_DEV_INVALID == uartDev) {
    return;
  }
  struct uartInfo* uartInfo = &interfaces[uartDev];

  if (!uartInfo->isEnabled) {
    return;
  }

  BaseType_t higherPriorityTaskWoken = pdFALSE;
  if (uartInfo->outputSbEnabled) {
    xStreamBufferSendFromISR(uartInfo->outputSb, uartInfo->uartDmaRx, size,
                             &higherPriorityTaskWoken);
  }

  // start the next DMA transfer
  HAL_UARTEx_ReceiveToIdle_DMA(huart, uartInfo->uartDmaRx, UART_MAX_DMA_LEN);

  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

/**
 * @brief UART DMA Tx complete interrupt
 * 
 * @param huart UART handle provided by interrupt
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart)
{
  UART_Device_T uartDev = uartHandleToDevice(huart);
  if (UART_DEV_INVALID == uartDev) {
    return;
  }
  struct uartInfo* uartInfo = &interfaces[uartDev];

  if (!uartInfo->isEnabled) {
    return;
  }

  BaseType_t higherPriorityTaskWoken = pdFALSE;

  // check whether there is more data to transmit
  if (pdTRUE == xStreamBufferIsEmpty(uartInfo->txPendingStreamHandle)) {
    uartInfo->txInProgress = false;
  } else {
    uint16_t numSending = (uint16_t)xStreamBufferReceiveFromISR(
        uartInfo->txPendingStreamHandle, uartInfo->uartDmaTx, UART_MAX_DMA_LEN,
        &higherPriorityTaskWoken);

    HAL_StatusTypeDef txStatus = HAL_UART_Transmit_DMA(
        huart, uartInfo->uartDmaTx, numSending);
    if (HAL_OK != txStatus) {
      // drop the failed bytes
      uartInfo->txInProgress = false;
    }
  }

  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  UART_Device_T uartDev = uartHandleToDevice(huart);
  if (UART_DEV_INVALID == uartDev) {
    return;
  }
  struct uartInfo* uartInfo = &interfaces[uartDev];

  if (!uartInfo->isEnabled) {
    return;
  }

  uartInfo->txInProgress = false;

  HAL_UARTEx_ReceiveToIdle_DMA(huart, uartInfo->uartDmaRx, UART_MAX_DMA_LEN);
}


// ------------------- Public methods -------------------
UART_Status_T UART_Init(Logging_T* logger)
{
  mLog = logger;
  Log_Print(mLog, "UART_Init begin\n");
  DEPEND_ON(logger, UART_STATUS_ERROR_DEPENDS);

  memset(interfaces, 0, sizeof(interfaces));
  // Individual device initialization is done in UART_Config

  REGISTER_STATIC(UART, UART_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "UART_Init complete\n");
  return UART_STATUS_OK;
}

//------------------------------------------------------------------------------
UART_Status_T UART_Config(UART_DeviceConfig_T* devConfig)
{
  Log_Print(mLog, "UART_Config begin\n");
  DEPEND_ON_STATIC(UART, UART_STATUS_ERROR_DEPENDS);

  UART_Device_T uartDev = uartHandleToDevice(devConfig->handle);
  if (UART_DEV_INVALID == uartDev) {
    return UART_STATUS_ERROR_INVALID_DEV;
  }
  struct uartInfo* uartInfo = &interfaces[uartDev];

  if (uartInfo->isEnabled) {
    return UART_STATUS_ERROR_INVALID_DEV;
  }

  // Check for consistent interfaces
  if (uartDev != devConfig->dev) {
    return UART_STATUS_ERROR_INVALID_DEV;
  }

  // Init this UART interface
  memset(uartInfo, 0, sizeof(struct uartInfo));
  uartInfo->handle = devConfig->handle;
  uartInfo->rxIrq = devConfig->rxIrq;
  uartInfo->txPendingStreamHandle = xStreamBufferCreateStatic(
      UART_MAX_DMA_LEN,
      1U,
      uartInfo->txPendingStorage,
      &uartInfo->txPendingStreamStruct);
  uartInfo->isEnabled = true;

  // Start receiving
  HAL_UARTEx_ReceiveToIdle_DMA(uartInfo->handle, uartInfo->uartDmaRx, UART_MAX_DMA_LEN);

  Log_Print(mLog, "UART_Config complete\n");
  return UART_STATUS_OK;
}

//------------------------------------------------------------------------------
UART_Status_T UART_SetRecvStream(
    const UART_Device_T dev,
    const StreamBufferHandle_t sb)
{
  if (dev >= UART_NUM_INTERFACES) {
    return UART_STATUS_ERROR_INVALID_DEV;
  }
  struct uartInfo* uartInfo = &interfaces[dev];

  if (!uartInfo->isEnabled) {
    return UART_STATUS_NOT_READY;
  }

  uartInfo->outputSb = sb;
  uartInfo->outputSbEnabled = true;

  return UART_STATUS_OK;
}

//------------------------------------------------------------------------------
UART_Status_T UART_SendMessage(const UART_Device_T dev, uint8_t* data, uint16_t len)
{
  if (dev >= UART_NUM_INTERFACES) {
    return UART_STATUS_ERROR_INVALID_DEV;
  }
  struct uartInfo* uartInfo = &interfaces[dev];

  if (!uartInfo->isEnabled) {
    return UART_STATUS_NOT_READY;
  }

  UART_Status_T ret = UART_STATUS_OK;

  // disable HAL_UART_TxCpltCallback to make this section atomic
  HAL_NVIC_DisableIRQ(uartInfo->rxIrq);

  if (uartInfo->txInProgress) {
    // queue for when hardware is done
    xStreamBufferSend(uartInfo->txPendingStreamHandle, (void*)data, len, pdMS_TO_TICKS(100U));
  } else {
    // directly initiate transfer
    uartInfo->txInProgress = true;
    memcpy(uartInfo->uartDmaTx, data, len);

    HAL_StatusTypeDef txStatus = HAL_UART_Transmit_DMA(uartInfo->handle, uartInfo->uartDmaTx, len);
    if (HAL_OK != txStatus) {
      uartInfo->txInProgress = false;
      ret = UART_STATUS_ERROR_TX;
    }
  }

  // atomic section complete
  HAL_NVIC_EnableIRQ(uartInfo->rxIrq);

  return ret;
}
