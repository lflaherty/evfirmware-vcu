/*
 * spi.c
 *
 *  Created on: Apr 20, 2021
 *      Author: Liam Flaherty
 */

#include "spi.h"
#include "stm32f7xx_hal.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

// ------------------- Private data -------------------

/* ========= SPI devices definitions ========= */
// Array of callbacks
#define SPI_NUM_BUSSES 4

// Indices for spiDevices
typedef enum
{
  SPI1_IDX       = 0,
  SPI2_IDX       = 1,
  SPI3_IDX       = 2,
  SPI4_IDX       = 3,
  SPI_INVALID_IDX = 127
} SPI_Index_T;

// SPI_Device_T but with some internal fields
typedef struct {
  SPI_Device_T* device;

  SemaphoreHandle_t mutex;      // Used to protect device while access is attempted
  StaticSemaphore_t mutexBuffer; // Buffer for static mutex
} SPI_Device_Internal_T;

// Stores the SPI device/CS pin/callback info for currently in use devices
static SPI_Device_Internal_T spiDevices[SPI_NUM_BUSSES];


/* ========= Rx Task definitions ========= */
static struct {
  // Task handle for Rx task
  TaskHandle_t spiTaskHandle;

  // Holds the TCB for the spi Rx callback thread
  StaticTask_t xTaskBuffer;

  // Callback thread will this this as it's stack
  StackType_t xTask[SPI_STACK_SIZE];
} spiBusTask;

/* ========= ISR -> Thread queue ========= */
#define SPI_QUEUE_ITEM_SIZE   sizeof(SPI_Index_T)

static struct {
  // Handle for queue
  QueueHandle_t spiDataQueue;

  // Used to hold queue's data structure
  StaticQueue_t spiDataStaticQueue;

  // Used as the queue's storage area.
  uint8_t spiDataQueueStorageArea[SPI_QUEUE_LENGTH*SPI_QUEUE_ITEM_SIZE];
} spiBusQueue;

/* ========= Data for synchronous call ========= */
static struct {
  // mutex for locking SPI_TransmitReceiveBlocking
  SemaphoreHandle_t spiBusyMutex;
  StaticSemaphore_t spiBusyMutexBuffer;

  // binary semaphore for signaling from the SPI callback to SPI_TransmitReceiveBlocking
  SemaphoreHandle_t signalSem;
  StaticSemaphore_t signalSemBuffer;

  uint8_t rxData[SPI_SYNC_MAX_DATA_LENGTH]; // TODO should this be volatile?
  uint8_t txData[SPI_SYNC_MAX_DATA_LENGTH];
} spiSync;




// ------------------- Private methods -------------------
/**
 * Converts the SPIx bus instance to an index for spiDevices
 */
static SPI_Index_T getSPIBusIndex(SPI_TypeDef* spiBus) {
  // find the correct index for this bus instance
  if (spiBus == SPI1) { return SPI1_IDX; }
  else if (spiBus == SPI2) { return SPI2_IDX; }
  else if (spiBus == SPI3) { return SPI3_IDX; }
  else if (spiBus == SPI4) { return SPI4_IDX; }
  else {
    // shouldn't have reached here...
    return SPI_INVALID_IDX;
  }
}

/**
 * Task code for SPI Rx callback thread
 */
static void SPI_RxTask(void* pvParameters)
{
  printf("SPI_RxTask begin\n");

  const TickType_t blockTime = 500 / portTICK_PERIOD_MS; // 500ms
  uint32_t notifiedValue;

  while (1) {
    // wait for notification from ISR
    notifiedValue = ulTaskNotifyTake(pdFALSE, blockTime);

    while (notifiedValue > 0) {
      // process callbacks

      // Receive data from the queue (and don't block)

      // The callback gives us the index of the currently in use device
      SPI_Index_T spiDevIndex;
      BaseType_t recvStatus = xQueueReceive(spiBusQueue.spiDataQueue, &spiDevIndex, 0);

      if (recvStatus == pdTRUE) {
        // Call the SPI callback methods

        // No need to acquire the binary semaphore - it is taken in the TxRx method
        // and not given back until after the callback

        // call the callback
        // no data needed as the DMA call put the
        if (NULL != spiDevices[spiDevIndex].device->callback) {
          spiDevices[spiDevIndex].device->callback();
        }

        // pend semaphore (signal that the device is ready again)
        xSemaphoreGive(spiDevices[spiDevIndex].mutex);

        notifiedValue--; // one less notification to process
      } else {
        break; // exit loop processing all notifications
      }

    }
  }
}

/**
 * Method to synchronize the synchronous TX call SPI_TransmitReceiveBlocking
 */
static void SPI_TransmitReceiveBlocking_Callback(void)
{
  // Give semaphore to SPI_TransmitReceiveBlocking
  xSemaphoreGive(spiSync.signalSem);
}

//------------------------------------------------------------------------------
/**
 * @brief SPI Rx interrupt
 *
 * @brief hspi SPI handle provided by interrupt
 */
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi)
{
  SPI_Index_T spiDevIndex = getSPIBusIndex(hspi->Instance);
  if (spiDevIndex == SPI_INVALID_IDX) {
    return;
  }

  // pull up CS pin
  HAL_GPIO_WritePin(
      spiDevices[spiDevIndex].device->csPinBank,
      spiDevices[spiDevIndex].device->csPin,
      GPIO_PIN_SET);

  // push spiIndex to the complete queue & notify
  BaseType_t status = xQueueSendToBackFromISR(spiBusQueue.spiDataQueue, &spiDevIndex, NULL);

  // only notify if adding to the queue worked
  if (status == pdPASS) {
    // Notify waiting thread
    vTaskNotifyGiveFromISR(spiBusTask.spiTaskHandle, NULL);
  }
}



// ------------------- Public methods -------------------
SPI_Status_T SPI_Init(void)
{
  printf("SPI_Init begin\n");

  // Initialize spiDevices
  for (uint8_t i = 0; i < SPI_NUM_BUSSES; ++i) {
    // invalidate device and create mutex
    spiDevices[i].device = NULL;
    spiDevices[i].mutex = xSemaphoreCreateBinaryStatic(&spiDevices[i].mutexBuffer);
//    spiDevices[i].mutex =
//        xSemaphoreCreateMutexStatic(&spiDevices[i].mutexBuffer);

    // start semaphore as available
    xSemaphoreGive(spiDevices[i].mutex);
    // TODO rename mutex to sem

    if(spiDevices[i].mutex == NULL) {
      // error in creating mutex semaphore
      // created static, so not expected, but still check...
      return SPI_STATUS_ERROR_SEM;
    }
  }

  // Create mutex for SPI_TransmitReceiveBlocking
  spiSync.spiBusyMutex = xSemaphoreCreateMutexStatic(&spiSync.spiBusyMutexBuffer);
  if (NULL == spiSync.spiBusyMutex) {
    return SPI_STATUS_ERROR_SEM;
  }

  // Create semaphore for SPI_TransmitReceiveBlocking synchronization
  spiSync.signalSem = xSemaphoreCreateBinaryStatic(&spiSync.signalSemBuffer);
  if (NULL == spiSync.signalSem) {
    return SPI_STATUS_ERROR_SEM;
  }
  // initialize semaphore to available
  if (pdTRUE != xSemaphoreGive(spiSync.signalSem)) {
    return SPI_STATUS_ERROR_SEM;
  }

  // create the ISR -> task data queue
  spiBusQueue.spiDataQueue = xQueueCreateStatic(
      SPI_QUEUE_LENGTH,
      SPI_QUEUE_ITEM_SIZE,
      spiBusQueue.spiDataQueueStorageArea,
      &spiBusQueue.spiDataStaticQueue);

  // create thread for processing the callbacks outside of an interrupt
  spiBusTask.spiTaskHandle = xTaskCreateStatic(
      SPI_RxTask,
      "SPI_RxCallback",
      SPI_STACK_SIZE,
      NULL,               // Parameter passed into the task (none in this case)
      tskIDLE_PRIORITY,  // TODO: priority?
      spiBusTask.xTask,
      &spiBusTask.xTaskBuffer);

  printf("SPI_Init complete\n");
  return SPI_STATUS_OK;
}

//------------------------------------------------------------------------------
SPI_Status_T SPI_TransmitReceive(
    SPI_Device_T* device,
    uint8_t* txData,
    uint8_t* rxData,
    uint16_t dataLen)
{
  // get index for spiBusTransfers
  SPI_Index_T spiDevIndex = getSPIBusIndex(device->spiHandle->Instance);
  if (spiDevIndex == SPI_INVALID_IDX) {
    return SPI_STATUS_ERROR_INVALID_BUS;
  }

  // Acquire mutex
  // TODO timeout
  if (xSemaphoreTake(spiDevices[spiDevIndex].mutex, (TickType_t)10) != pdTRUE) {
    // could not obtain the semaphore and cannot access resource
    return SPI_STATUS_ERROR_BUSY;
  }

  // assign the device
  spiDevices[spiDevIndex].device = device;

  // Pull CS pin low to begin transfer
  HAL_GPIO_WritePin(device->csPinBank, device->csPin, GPIO_PIN_RESET);

  // TODO move from IT to DMA
//  HAL_StatusTypeDef ret = HAL_SPI_TransmitReceive_DMA(
//      device->spiHandle, txData, rxData, dataLen);
  HAL_StatusTypeDef ret = HAL_SPI_TransmitReceive_IT(
      device->spiHandle, txData, rxData, dataLen);
  if (HAL_OK != ret) {
    // failed transfer

    // Pull CS pin back up
    HAL_GPIO_WritePin(device->csPinBank, device->csPin, GPIO_PIN_SET);

    return SPI_STATUS_ERROR_TX;
  }

  return SPI_STATUS_OK;
}

//------------------------------------------------------------------------------
SPI_Status_T SPI_TransmitReceiveBlocking(
    SPI_Device_T* device,
    uint8_t* txData,
    uint8_t* rxData,
    uint16_t dataLen)
{
  if (dataLen > SPI_SYNC_MAX_DATA_LENGTH) {
    return SPI_STATUS_ERROR_TOO_MUCH_DATA;
  }

  // Pull CS pin low to begin transfer
  HAL_GPIO_WritePin(device->csPinBank, device->csPin, GPIO_PIN_RESET);

  HAL_StatusTypeDef x = HAL_SPI_TransmitReceive(device->spiHandle, txData, rxData, dataLen, 10U);

  // Pull CS pin back up
  HAL_GPIO_WritePin(device->csPinBank, device->csPin, GPIO_PIN_SET);

  if (HAL_OK != x) {
    return SPI_STATUS_ERROR_TX;
  }

//  // grab the mutex
//  if (pdTRUE != xSemaphoreTake(spiSync.spiBusyMutex, 10U)) {
//    return SPI_STATUS_ERROR_SEM;
//  }
//
//  device->callback = SPI_TransmitReceiveBlocking_Callback;
//  SPI_Status_T ret = SPI_TransmitReceive(device, spiSync.txData, spiSync.rxData, dataLen);
//  if (SPI_STATUS_OK != ret) {
//    // release the semaphore
//    xSemaphoreGive(spiSync.spiBusyMutex);
//
//    // error
//    return ret;
//  }
//
//  // wait for the synchronizing semaphore
//  if (pdTRUE != xSemaphoreTake(spiSync.signalSem, 10U)) {
//    // release the semaphore
//    xSemaphoreGive(spiSync.spiBusyMutex);
//
//    return SPI_STATUS_ERROR_SEM;
//  }
//
//  // copy data from volatile arrays into output
//  // TODO uint16_ts?
//  memcpy(txData, spiSync.txData, sizeof(uint8_t)*dataLen);
//  memcpy(rxData, spiSync.rxData, sizeof(uint8_t)*dataLen);
//
//  // now release data mutex
//  xSemaphoreGive(spiSync.spiBusyMutex);

  return SPI_STATUS_OK;
}

