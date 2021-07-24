/*
 * uart.h
 *
 *  Created on: May 23, 2021
 *      Author: Liam Flaherty
 */

#ifndef COMM_UART_UART_H_
#define COMM_UART_UART_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>

#define UART_QUEUE_LENGTH 128     /* 128 bytes */
#define UART_NUM_CALLBACKS 16     /* Max number of UART callbacks on any device */

/**
 * Stack size for UART Rx callback thread.
 * Note the units of this: words
 * The STM32 has a 32-bit word size.
 * I.e. 200 word stack size => 200*32bit = 800 Bytes
 * This will be the same stack in the call-back methods
 */
#define UART_STACK_SIZE 200

typedef enum
{
  UART_STATUS_OK        = 0x00U,
  UART_STATUS_ERROR_TX  = 0x01U,
  UART_STATUS_ERROR_CALLBACK_FULL = 0x02U
} UART_Status_T;

typedef struct {
  USART_TypeDef* uartInstance;
  uint8_t data;
} USART_Data_T;

/**
 * @brief Callback method typedef
 * Params:
 *    New USART data byte
 */
typedef void (*UART_Callback_Method)(const USART_Data_T*);

/**
 * @brief Initialize UART driver interface
 */
UART_Status_T UART_Init(void);

/**
 * @brief Configure UART bus
 * This should be called from main. Main will retain ownership of handle ptr.
 *
 * @return Return status. UART_STATUS_OK for success. See UART_Status_T for more.
 */
UART_Status_T UART_Config(UART_HandleTypeDef* handle);

/**
 * @biref Adds a method to the callback list. Method will be invoked when a
 * UART byte is received.
 *
 * @param handle UART device handle
 * @param callback Method to call during callback
 * @return Return status. UART_STATUS_OK for success. See UART_Status_T for more.
 */
UART_Status_T UART_RegisterCallback(
    const UART_HandleTypeDef* handle,
    const UART_Callback_Method callback);

/**
 * @brief Send a serial message
 *
 * @param handle UARTdevice handle
 * @param data Array of data to send
 * @param n Length of data array
 * @return Return status. UART_STATUS_OK for success. See UART_Status_T for more.
 */
UART_Status_T UART_SendMessage(UART_HandleTypeDef* handle, uint8_t* data, size_t len);


#endif /* COMM_UART_UART_H_ */
