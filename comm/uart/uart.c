/*
 * uart.c
 *
 *  Created on: May 23, 2021
 *      Author: Liam Flaherty
 */

#include "uart.h"

#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// ------------------- Private data -------------------
typedef struct {
  USART_TypeDef* uartInstance;
  UART_Callback_Method callback;
} UART_Callback_T;

static struct {
  uint8_t numCallbacks;   // stores how many callbacks are currently registered
  UART_Callback_T callbacks[UART_NUM_CALLBACKS];
} uartInfo;

static uint8_t uartDmaData[2];

/* ========= Rx Task definitions ========= */
static struct {
  // Task handle for Rx task
  TaskHandle_t uartTaskHandle;

  // Holds the TCB for the UART Rx callback thread
  StaticTask_t xTaskBuffer;

  // Callback thread will this this as it's stack
  StackType_t xTask[UART_STACK_SIZE];
} uartRxTask;

/* ========= ISR -> Thread queue ========= */
#define UART_QUEUE_ITEM_SIZE   sizeof(USART_Data_T)

static struct {
  // Handle for queue
  QueueHandle_t uartDataQueue;

  // Used to hold queue's data structure
  StaticQueue_t uartDataStaticQueue;

  // Used as the queue's storage area.
  uint8_t uartDataQueueStorageArea[UART_QUEUE_LENGTH*UART_QUEUE_ITEM_SIZE];
} uartQueue;

// ------------------- Private methods -------------------
/**
 * Task code for UART Rx callback thread
 */
static void UART_RxTask(void* pvParameters)
{
  const TickType_t blockTime = 500 / portTICK_PERIOD_MS; // 500ms
  uint32_t notifiedValue;

  while (1) {
    // wait for notification from ISR
    notifiedValue = ulTaskNotifyTake(pdFALSE, blockTime);

    while (notifiedValue > 0) {
      // process callbacks

      // Receive data from the queue (and don't block)
      USART_Data_T uartData;
      BaseType_t recvStatus = xQueueReceive(uartQueue.uartDataQueue, &uartData, 0);

      if (recvStatus == pdTRUE) {
        // Call the UART callback methods

        uint8_t numCallbacks = uartInfo.numCallbacks;
        for (uint8_t i = 0; i < numCallbacks; ++i) {
          // check the UART instance
          if (uartInfo.callbacks[i].uartInstance == uartData.uartInstance) {
            // invoke the callback
            // passing the pointer to local variable is ok -
            // it will exist in the local stack frame for the life of the callback
            uartInfo.callbacks[i].callback(&uartData);
          }
        }

        notifiedValue--; // one less notification to process
      } else {
        break; // exit loop processing all notifications
      }

    }
  }
}

/**
 * @brief UART DMA Rx interrupt
 * First byte
 *
 * @brief huart UART handle provided by interrupt
 */
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef* huart)
{
  // Get data
  USART_Data_T uartData;
  uartData.uartInstance = huart->Instance;
  uartData.data = uartDmaData[0];

  // add to queue and notify
  BaseType_t status = xQueueSendToBackFromISR(uartQueue.uartDataQueue, &uartData, NULL);
  if (pdPASS == status) {
    vTaskNotifyGiveFromISR(uartRxTask.uartTaskHandle, NULL);
  }
}

/**
 * @brief UART DMA Rx interrupt
 * Second byte
 *
 * @brief huart UART handle provided by interrupt
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
  // Get data
  USART_Data_T uartData;
  uartData.uartInstance = huart->Instance;
  uartData.data = uartDmaData[1];

  // add to queue and notify
  BaseType_t status = xQueueSendToBackFromISR(uartQueue.uartDataQueue, &uartData, NULL);
  if (pdPASS == status) {
    vTaskNotifyGiveFromISR(uartRxTask.uartTaskHandle, NULL);
  }

  // start the next DMA transfer
  HAL_UART_Receive_DMA(huart, uartDmaData, 2);
  // TODO is this needed?
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_DeInit(huart);
    HAL_UART_Receive_DMA(huart, uartDmaData, 2);
}


// ------------------- Public methods -------------------
UART_Status_T UART_Init(void)
{
  printf("UART_Init begin\n");

  // Initialize mem to 0
  memset(&uartInfo, 0, sizeof(uartInfo));

  // Create the ISR -> task data queue
  uartQueue.uartDataQueue = xQueueCreateStatic(
      UART_QUEUE_LENGTH,
      UART_QUEUE_ITEM_SIZE,
      uartQueue.uartDataQueueStorageArea,
      &uartQueue.uartDataStaticQueue);

  // create thread for processing the callbacks outside of an iterrupt
  uartRxTask.uartTaskHandle = xTaskCreateStatic(
      UART_RxTask,
      "UART_RXCallback",
      UART_STACK_SIZE,
      NULL,                 // Parameters to pass to the task
      tskIDLE_PRIORITY,     // TODO update priority
      uartRxTask.xTask,
      &uartRxTask.xTaskBuffer);

  printf("UART_Init complete\n");
  return UART_STATUS_OK;
}

//------------------------------------------------------------------------------
UART_Status_T UART_Config(UART_HandleTypeDef* handle)
{
  printf("UART_Config begin\n");

  HAL_UART_Receive_DMA(handle, uartDmaData, 2);
  // TODO is this needed?

  printf("UART_Config complete\n");
  return UART_STATUS_OK;
}

//------------------------------------------------------------------------------
UART_Status_T UART_RegisterCallback(
    const UART_HandleTypeDef* handle,
    const UART_Callback_Method method)
{
  // Store callback
  if (UART_NUM_CALLBACKS == uartInfo.numCallbacks) {
    // the callback array is already full
    return UART_STATUS_ERROR_CALLBACK_FULL;
  }

  // before incrementing, numCallbacks stores the next index we could add to
  uartInfo.callbacks[uartInfo.numCallbacks].uartInstance = handle->Instance;
  uartInfo.callbacks[uartInfo.numCallbacks].callback = method;
  uartInfo.numCallbacks++;

  return UART_STATUS_OK;
}

//------------------------------------------------------------------------------
UART_Status_T UART_SendMessage(UART_HandleTypeDef* handle, uint8_t* data, size_t len)
{
  HAL_StatusTypeDef ret = HAL_UART_Transmit_DMA(handle, data, len);
  if (HAL_OK != ret) {
    return UART_STATUS_ERROR_TX;
  }

  return UART_STATUS_OK;
}
