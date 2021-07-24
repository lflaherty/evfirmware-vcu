/*
 * can.c
 *
 *  Created on: Oct 22, 2020
 *      Author: Liam Flaherty
 */

#include "can.h"

#include <string.h>
#include <stdio.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// ------------------- Private data -------------------

/* ========= CAN bus definitions ========= */

typedef struct {
  CAN_TypeDef* busInstance;       // The callback is tied to this CAN Bus instance
  uint32_t msgId;                 // Message ID must match for callback to be invoked
  CAN_Callback_Method callback;   // The method to invoke in the callback
} CAN_Callback_T;

/**
 * @brief CAN Bus storage
 */
static struct {
  uint32_t txMailbox;

  uint8_t numCallbacks;  // stores how many callbacks are currently registered
  CAN_Callback_T callbacks[CAN_NUM_CALLBACKS];
} canBusInfo;

/* ========= Rx Task definitions ========= */
static struct {
  // Task handle for Rx task
  TaskHandle_t canTaskHandle;

  // Holds the TCB for the CAN Rx callback thread
  StaticTask_t xTaskBuffer;

  // Callback thread will this this as it's stack
  StackType_t xTask[CAN_STACK_SIZE];
} canBusTask;

/* ========= ISR -> Thread queue ========= */
#define CAN_QUEUE_ITEM_SIZE   sizeof(CAN_DataFrame_T)

static struct {
  // Handle for queue
  QueueHandle_t canDataQueue;

  // Used to hold queue's data structure
  StaticQueue_t canDataStaticQueue;

  // Used as the queue's storage area.
  uint8_t canDataQueueStorageArea[CAN_QUEUE_LENGTH*CAN_QUEUE_ITEM_SIZE];
} canBusQueue;


// ------------------- Private methods -------------------
/**
 * Task code for CAN Rx callback thread
 */
static void CAN_RxTask(void* pvParameters)
{
  const TickType_t blockTime = 500 / portTICK_PERIOD_MS; // 500ms
  uint32_t notifiedValue;

  while (1) {
    // wait for notification from ISR
    notifiedValue = ulTaskNotifyTake(pdFALSE, blockTime);

    while (notifiedValue > 0) {
      // process callbacks

      // Receive data from the queue (and don't block)
      CAN_DataFrame_T canData;
      BaseType_t recvStatus = xQueueReceive(canBusQueue.canDataQueue, &canData, 0);

      if (recvStatus == pdTRUE) {
        // Call the CAN callback methods

        uint8_t numCallbacks = canBusInfo.numCallbacks;
        for (uint8_t i = 0; i < numCallbacks; ++i) {
          // check the CAN bus instance & the message ID
          if (canBusInfo.callbacks[i].busInstance == canData.busInstance &&
              canBusInfo.callbacks[i].msgId == canData.msgId) {
            // invoke the callback
            // passing the pointer to local variable is ok -
            // it will exist in the local stack frame for the life of the callback
            canBusInfo.callbacks[i].callback(&canData);
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
 * @brief CAN Rx interrupt
 *
 * @brief hcan CAN Bus handle provided by interrupt
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
  CAN_RxHeaderTypeDef rxHeader;
  CAN_DataFrame_T canData;

  /* Get RX message */
  if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &rxHeader, canData.data) != HAL_OK) {
    return;
  }

  // Add data to queue and notify waiting thread
  canData.busInstance = hcan->Instance;
  canData.msgId = rxHeader.StdId;
  canData.dlc = rxHeader.DLC;
  // (canData.data directly assigned from HAL_CAN_GetRxMessage)

  BaseType_t status = xQueueSendToBackFromISR(canBusQueue.canDataQueue, &canData, NULL);

  // only notify if adding to the queue worked
  if (status == pdPASS) {
    // Notify waiting thread
    vTaskNotifyGiveFromISR(canBusTask.canTaskHandle, NULL);
  }
}

// ------------------- Public methods -------------------
CAN_Status_T CAN_Init(void)
{
  printf("CAN_Init begin\n");
  // Initialize mem to 0
  memset(&canBusInfo, 0, sizeof(canBusInfo));

  // create the ISR -> task data queue
  canBusQueue.canDataQueue = xQueueCreateStatic(
      CAN_QUEUE_LENGTH,
      CAN_QUEUE_ITEM_SIZE,
      canBusQueue.canDataQueueStorageArea,
      &canBusQueue.canDataStaticQueue);

  // create thread for processing the callbacks outside of an interrupt
  canBusTask.canTaskHandle = xTaskCreateStatic(
      CAN_RxTask,
      "CAN_RxCallback",
      CAN_STACK_SIZE,
      NULL,               // Parameter passed into the task (none in this case)
      tskIDLE_PRIORITY,  // TODO: priority?
      canBusTask.xTask,
      &canBusTask.xTaskBuffer);

  printf("CAN_Init complete\n");
  return CAN_STATUS_OK;
}

//------------------------------------------------------------------------------
// Rename to CAN_Start
CAN_Status_T CAN_Config(CAN_HandleTypeDef* handle)
{
  printf("CAN_Config begin\n");

  // Filter config
  CAN_FilterTypeDef  sFilterConfig;

  sFilterConfig.FilterBank = 0;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = 14;

  if (HAL_CAN_ConfigFilter(handle, &sFilterConfig) != HAL_OK) {
    return CAN_STATUS_ERROR_CFG_FILTER;
  }

  // Start bus
  if (HAL_CAN_Start(handle) != HAL_OK) {
    return CAN_STATUS_ERROR_START;
  }

  //Activate CAN RX interrupt
  if (HAL_CAN_ActivateNotification(handle, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK) {
    return CAN_STATUS_ERROR_START_NOTIFY;
  }

  printf("CAN_Config complete\n");
  return CAN_STATUS_OK;
}

//------------------------------------------------------------------------------
CAN_Status_T CAN_RegisterCallback(
    const CAN_HandleTypeDef* handle,
    const uint32_t msgId,
    const CAN_Callback_Method method)
{
  // Store callback
  if (canBusInfo.numCallbacks == CAN_NUM_CALLBACKS) {
    // the callback array is already full
    return CAN_STATUS_ERROR_CALLBACK_FULL;
  }

  // before incrementing, numCallbacks stores the next index we could add to
  canBusInfo.callbacks[canBusInfo.numCallbacks].busInstance = handle->Instance;
  canBusInfo.callbacks[canBusInfo.numCallbacks].msgId = msgId;
  canBusInfo.callbacks[canBusInfo.numCallbacks].callback = method;
  canBusInfo.numCallbacks++;

  return CAN_STATUS_OK;
}

//------------------------------------------------------------------------------
CAN_Status_T CAN_SendMessage(CAN_HandleTypeDef* handle, uint32_t msgId, uint8_t* data, size_t n)
{
  // Construct header
  CAN_TxHeaderTypeDef txHeader;
  txHeader.StdId = msgId;
  txHeader.ExtId = msgId;
  txHeader.DLC = n;
  txHeader.RTR = CAN_RTR_DATA;
  txHeader.IDE = CAN_ID_STD; // Standard ID
  txHeader.TransmitGlobalTime = DISABLE;

  // Send
  HAL_StatusTypeDef err = HAL_CAN_AddTxMessage(handle, &txHeader, data, &canBusInfo.txMailbox);
  if (err != HAL_OK) {
    return CAN_STATUS_ERROR_TX;
  }

  return CAN_STATUS_OK;
}

