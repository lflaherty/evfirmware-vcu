/*
 * can.c
 *
 *  Created on: Oct 22, 2020
 *      Author: Liam Flaherty
 */

#include "can.h"

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

REGISTERED_MODULE_STATIC_DEF(CAN);

// ------------------- Private data -------------------
static Logging_T* mLog;

/* ========= CAN bus definitions ========= */
struct CAN_RecvQueue {
  uint32_t deviceId;
  uint32_t deviceIdMask;
  QueueHandle_t queue;
};

typedef struct {
  CAN_TxHeaderTypeDef header;
  uint8_t data[8];
} TxPendingItem_T;

#define TX_PENDING_ITEM_SIZE (sizeof(TxPendingItem_T))
#define TX_PENDING_SORAGE_SIZE (CAN_MAX_PENDING_MSGS * TX_PENDING_ITEM_SIZE)

/**
 * @brief CAN Bus storage
 */
struct CAN_Instance {
  bool inUse;

  CAN_HandleTypeDef* handle;

  uint8_t numQueues;  // stores how many callbacks are currently registered
  struct CAN_RecvQueue queues[CAN_MAX_RECV_QUEUES];

  // tx buffer, stores TxPendingItem_T objects
  QueueHandle_t txQueueHandle;
  StaticQueue_t txQueueBuffer;
  uint8_t txQueueStorageArea[TX_PENDING_SORAGE_SIZE];
};

static struct CAN_Instance canInstances[CAN_NUM_INSTANCES];

// ------------------- Private methods -------------------
/**
 * @brief CAN Rx interrupt for any fifo. Called by one of the other ISRs.
 *
 * @brief hcan CAN Bus handle provided by interrupt
 * @brief rxFifo RX FIFO object
 */
static void ISR_RxMsgPendingCallback(CAN_HandleTypeDef *hcan, const uint32_t rxFifo)
{
  CAN_Device_T canDevInstance;
  if (CAN1 == hcan->Instance) {
    canDevInstance = CAN_DEV1;
  } else if (CAN2 == hcan->Instance) {
    canDevInstance = CAN_DEV2;
  } else if (CAN3 == hcan->Instance) {
    canDevInstance = CAN_DEV3;
  } else {
    // Not implemented
    return;
  }
  struct CAN_Instance* canDev = &canInstances[canDevInstance];

  if (!canDev->inUse) {
    return;
  }

  CAN_RxHeaderTypeDef rxHeader;
  CAN_DataFrame_T canData;
  BaseType_t higherPriorityTaskWoken = pdFALSE;

  // check HAL_CAN_GetRxFifoFillLevel TODO
  while (HAL_CAN_GetRxFifoFillLevel(hcan, rxFifo) > 0) {
    /* Get RX message */
    if (HAL_CAN_GetRxMessage(hcan, rxFifo, &rxHeader, canData.data) != HAL_OK) {
      return;
    }

    canData.busInstance = canDevInstance;
    canData.msgId = rxHeader.StdId;
    canData.dlc = rxHeader.DLC;
    // (canData.data directly assigned from HAL_CAN_GetRxMessage)

    for (uint8_t i = 0; i < canDev->numQueues; ++i) {
      uint32_t deviceId = canDev->queues[i].deviceId;
      uint32_t deviceIdMask = canDev->queues[i].deviceIdMask;
      BaseType_t queueWokeHigherPriorityTask = pdFALSE;
      if ((canData.msgId & deviceIdMask) == deviceId) {
        xQueueSendToBackFromISR(canDev->queues[i].queue, &canData, &queueWokeHigherPriorityTask);
      }

      if (queueWokeHigherPriorityTask) {
        higherPriorityTaskWoken = pdTRUE;
      }
    }
  }

  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

/**
 * @brief CAN Rx interrupt for FIFO0
 *
 * @brief hcan CAN Bus handle provided by interrupt
 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan)
{
  ISR_RxMsgPendingCallback(hcan, CAN_RX_FIFO0);
}

/**
 * @brief CAN Rx interrupt for FIFO1
 *
 * @brief hcan CAN Bus handle provided by interrupt
 */
void HAL_CAN_RxFifo1MsgPendingCallback(CAN_HandleTypeDef* hcan)
{
  ISR_RxMsgPendingCallback(hcan, CAN_RX_FIFO1);
}

void ISR_TxCompleteCallback(CAN_HandleTypeDef* hcan, const uint32_t mailbox)
{
  (void)mailbox;

  CAN_Device_T canDevInstance;
  if (CAN1 == hcan->Instance) {
    canDevInstance = CAN_DEV1;
  } else if (CAN2 == hcan->Instance) {
    canDevInstance = CAN_DEV2;
  } else if (CAN3 == hcan->Instance) {
    canDevInstance = CAN_DEV3;
  } else {
    // Not implemented
    return;
  }
  struct CAN_Instance* canDev = &canInstances[canDevInstance];

  if (!canDev->inUse) {
    return;
  }

  BaseType_t higherPriorityTaskWoken = pdFALSE;

  while (!xQueueIsQueueEmptyFromISR(canDev->txQueueHandle) &&
         HAL_CAN_GetTxMailboxesFreeLevel(canDev->handle) > 0) {
    // Have a message pending and a free mailbox
    BaseType_t xTaskWokenByReceive = pdFALSE;
    TxPendingItem_T txItem;
    if (xQueueReceiveFromISR(canDev->txQueueHandle, (void*)&txItem, &xTaskWokenByReceive)) {
      uint32_t newMailbox;
      HAL_CAN_AddTxMessage(canDev->handle, &txItem.header, txItem.data, &newMailbox);
    }

    if (xTaskWokenByReceive != pdFALSE) {
      higherPriorityTaskWoken = pdTRUE;
    }
  }

  portYIELD_FROM_ISR(higherPriorityTaskWoken);
}

void HAL_CAN_TxMailbox0CompleteCallback(CAN_HandleTypeDef* hcan)
{
  ISR_TxCompleteCallback(hcan, CAN_TX_MAILBOX0);
}

void HAL_CAN_TxMailbox1CompleteCallback(CAN_HandleTypeDef* hcan)
{
  ISR_TxCompleteCallback(hcan, CAN_TX_MAILBOX1);
}

void HAL_CAN_TxMailbox2CompleteCallback(CAN_HandleTypeDef* hcan)
{
  ISR_TxCompleteCallback(hcan, CAN_TX_MAILBOX2);
}

// ------------------- Public methods -------------------
CAN_Status_T CAN_Init(Logging_T* logger)
{
  mLog = logger;
  Log_Print(mLog, "CAN_Init begin\n");
  DEPEND_ON(logger, CAN_STATUS_ERROR_DEPENDS);

  // Initialize mem to 0
  memset(canInstances, 0, sizeof(canInstances));

  for (uint8_t i = 0; i < CAN_NUM_INSTANCES; ++i) {
    struct CAN_Instance* canDev = &canInstances[i];
    canDev->txQueueHandle = xQueueCreateStatic(
        CAN_MAX_PENDING_MSGS,
        TX_PENDING_ITEM_SIZE,
        canDev->txQueueStorageArea,
        &canDev->txQueueBuffer);
  }

  REGISTER_STATIC(CAN, CAN_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "CAN_Init complete\n");
  return CAN_STATUS_OK;
}

//------------------------------------------------------------------------------
CAN_Status_T CAN_Config(CAN_Device_T device, CAN_HandleTypeDef* handle)
{
  Log_Print(mLog, "CAN_Config begin ");
  DEPEND_ON_STATIC(CAN, CAN_STATUS_ERROR_DEPENDS);

  switch (device) {
    case CAN_DEV1:
      Log_Print(mLog, "CAN1\n");
      break;
    case CAN_DEV2:
      Log_Print(mLog, "CAN2\n");
      break;
    case CAN_DEV3:
      Log_Print(mLog, "CAN3\n");
      break;
    default:
      return CAN_STATUS_ERROR_INVALID_BUS;
  }

  struct CAN_Instance* canDev = &canInstances[device];

  canDev->handle = handle;
  canDev->inUse = true;

  // Filter config:
  // Assign each CAN interface to a dedicated filter bank.
  // Accept all incoming messages.
  CAN_FilterTypeDef sFilterConfig;

  sFilterConfig.FilterBank = device;
  sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK;
  sFilterConfig.FilterScale = CAN_FILTERSCALE_32BIT;
  sFilterConfig.FilterIdHigh = 0x0000;
  sFilterConfig.FilterIdLow = 0x0000;
  sFilterConfig.FilterMaskIdHigh = 0x0000;
  sFilterConfig.FilterMaskIdLow = 0x0000;
  sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0;
  sFilterConfig.FilterActivation = ENABLE;
  sFilterConfig.SlaveStartFilterBank = device;

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

  Log_Print(mLog, "CAN_Config complete\n");
  return CAN_STATUS_OK;
}

//------------------------------------------------------------------------------
CAN_Status_T CAN_RegisterQueue(
    const CAN_Device_T canInstance,
    const uint32_t deviceId,
    const uint32_t deviceIdMask,
    QueueHandle_t outQueue)
{
  if (canInstance >= CAN_NUM_INSTANCES) {
    return CAN_STATUS_ERROR_INVALID_BUS;
  }

  struct CAN_Instance* canDev = &canInstances[canInstance];
  uint8_t numQueues = canDev->numQueues;

  if (numQueues == CAN_MAX_RECV_QUEUES) {
    return CAN_STATUS_ERROR_MAX_QUEUES;
  }

  canDev->queues[numQueues].deviceId = deviceId;
  canDev->queues[numQueues].deviceIdMask = deviceIdMask;
  canDev->queues[numQueues].queue = outQueue;
  canDev->numQueues++;

  return CAN_STATUS_OK;
}

//------------------------------------------------------------------------------
CAN_Status_T CAN_SendMessage(
    const CAN_Device_T canInstance,
    uint32_t msgId,
    uint8_t* data,
    uint32_t n)
{
  if (canInstance >= CAN_NUM_INSTANCES) {
    return CAN_STATUS_ERROR_INVALID_BUS;
  }
  struct CAN_Instance* canDev = &canInstances[canInstance];

  // Construct header
  TxPendingItem_T txItem;
  txItem.header.StdId = msgId;
  txItem.header.ExtId = msgId;
  txItem.header.DLC = n;
  txItem.header.RTR = CAN_RTR_DATA;
  txItem.header.IDE = CAN_ID_STD; // Standard ID
  txItem.header.TransmitGlobalTime = DISABLE;

  CAN_Status_T status = CAN_STATUS_OK;

  // TODO turn off interrupts

  if (HAL_CAN_GetTxMailboxesFreeLevel(canDev->handle) > 0) {
    uint32_t mailbox = 0U;
    HAL_StatusTypeDef err = HAL_CAN_AddTxMessage(canDev->handle, &txItem.header, data, &mailbox);
    if (HAL_OK != err) {
      status = CAN_STATUS_ERROR_TX;
    }
  } else {
    memcpy(txItem.data, data, n);

    BaseType_t err = xQueueSendToBack(canDev->txQueueHandle, (void*)&txItem, (TickType_t)10U);
    if (pdTRUE != err) {
      status = CAN_STATUS_ERROR_TX;
    }
  }

  // TODO turn on interrupts

  return status;
}
