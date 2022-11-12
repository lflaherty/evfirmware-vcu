/*
 * uart.h
 *
 *  Created on: May 23, 2021
 *      Author: Liam Flaherty
 */

#ifndef COMM_UART_UART_H_
#define COMM_UART_UART_H_

#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include <stdint.h>

#include "lib/depends/depends.h"
#include "lib/logging/logging.h"

REGISTERED_MODULE_STATIC(UART);

#define UART_NUM_CALLBACKS 16     /* Max number of UART callbacks on any device */
#define UART_MAX_DMA_LEN 128   /* Max number of bytes in one message */

typedef enum
{
  UART_STATUS_OK                  = 0x00U,
  UART_STATUS_NOT_READY           = 0x01U,
  UART_STATUS_ERROR_TX            = 0x02U,
  UART_STATUS_ERROR_SB_FULL       = 0x03U,
  UART_STATUS_ERROR_NOT_SUPPORTED = 0x04U,
  UART_STATUS_ERROR_DEPENDS       = 0x05U,
} UART_Status_T;

/**
 * @brief Initialize UART driver interface
 * @param logger Pointer to logging settings
 */
UART_Status_T UART_Init(Logging_T* logger);

/**
 * @brief Configure UART bus
 * This should be called from main. Main will retain ownership of handle ptr.
 *
 * @return Return status. UART_STATUS_OK for success. See UART_Status_T for more.
 */
UART_Status_T UART_Config(UART_HandleTypeDef* handle);

/**
 * @biref Sets the output stream buffer for a particular UART device
 *
 * @param handle UART device handle
 * @param sb Stream buffer to send data to. (Note: This type is a pointer)
 * @return Return status. UART_STATUS_OK for success. See UART_Status_T for more.
 */
UART_Status_T UART_SetRecvStream(
    const UART_HandleTypeDef* handle,
    const StreamBufferHandle_t sb);

/**
 * @brief Send a serial message
 *
 * @param handle UARTdevice handle
 * @param data Array of data to send
 * @param n Length of data array
 * @return Return status. UART_STATUS_OK for success. See UART_Status_T for more.
 */
UART_Status_T UART_SendMessage(UART_HandleTypeDef* handle, uint8_t* data, uint16_t len);


#endif /* COMM_UART_UART_H_ */
