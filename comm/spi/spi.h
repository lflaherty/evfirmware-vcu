/*
 * spi.h
 *
 *  Created on: Apr 20, 2021
 *      Author: Liam Flaherty
 */

#ifndef COMM_SPI_SPI_H_
#define COMM_SPI_SPI_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "semphr.h"  /* FreeRTOS mutex */

#define SPI_QUEUE_LENGTH  10       /* 10 SPI messages */
#define SPI_NUM_CALLBACKS 5        /* Max number of SPI callbacks on any bus */

#define SPI_SYNC_MAX_DATA_LENGTH 16U  /* Max length of data messages transmitted using SPI_TransmitReceiveBlocking */

/**
 * Stack size for SPI Rx callback thread.
 * Note the units of this: words
 * The STM32 has a 32-bit word size.
 * I.e. 200 word stack size => 200*32bit = 800 Bytes
 * This will be the same stack in the call-back methods
 */
#define SPI_STACK_SIZE 200

typedef enum
{
  SPI_STATUS_OK                  = 0x00U,
  SPI_STATUS_ERROR_TX            = 0x01U,
  SPI_STATUS_ERROR_BUSY          = 0x02U,
  SPI_STATUS_ERROR_INVALID_BUS   = 0x03U,
  SPI_STATUS_ERROR_SEM           = 0x04U,
  SPI_STATUS_ERROR_TOO_MUCH_DATA = 0x05U
} SPI_Status_T;

/**
 * @brief Callback method typedef
 * Params:
 *    SPI data frame
 */
typedef void (*SPI_Callback_Method)(void);

/**
 * @brief Data structure used to control and SPI transfer and identify a callback
 */
typedef struct {
  // SPI definition and pins
  SPI_HandleTypeDef* spiHandle; // HAL handle for SPI device
  GPIO_TypeDef* csPinBank;      // GPIO bank used for CS pin
  uint16_t csPin;               // GPIO pin used for CS

  // Completion callback
  SPI_Callback_Method callback; // Triggered from HAL_SPI_TxRxCpltCallback interrupt
} SPI_Device_T;

/**
 * @brief Initialize SPI driver interface
 */
SPI_Status_T SPI_Init(void);


/**
 * @brief Initiates an asynchronous Transmit/Receive operation.
 * Operation will be completed using DMA.
 * Output data will be stored in *rxData and the callback registered to this
 * bus and CS pin will be raised.
 *
 * WARNING: Once invoked, *rxData could be modified at any time. Only use during callback. Mark as volatile.
 * WARNING: Recommend using a buffer array for *rxData, and processing/copying data from here only during callback.
 *
 * Caller must allocate memory.
 *
 * @param device struct representing the SPI handle, CS pin, and callback for the SPI device
 * @param txData pointer to data to transmit on SPI
 * @param rxData pointer to where data received from SPI will be stored
 * @param dataLen length of data arrays
 */
SPI_Status_T SPI_TransmitReceive(
    SPI_Device_T* device,
    uint8_t* txData,
    uint8_t* rxData,
    uint16_t dataLen);

/**
 * @brief Initiates a synchronous Transmit/Receive operation.
 * This will invoke SPI_TransmitReceive, but will pend (in FreeRTOS) until data is ready.
 * txData/rxData are not volatile from this call.
 *
 * device->callback is internal use only for this method
 * Do not allocate it, and do not use the variable later
 *
 * @param device struct representing the SPI handle, CS pin, and callback for the SPI device
 * @param txData pointer to data to transmit on SPI
 * @param rxData pointer to where data received from SPI will be stored
 * @param dataLen length of data arrays
 */
SPI_Status_T SPI_TransmitReceiveBlocking(
    SPI_Device_T* device,
    uint8_t* txData,
    uint8_t* rxData,
    uint16_t dataLen);

#endif /* COMM_SPI_SPI_H_ */
