/*
 * msgframeencode.h
 *
 * Usage:
 *   1. Create a MsgFrameEncode_T and set all fields.
 *   2. Call MsgFrameEncode_InitFrame to populate buffer.
 *   3. Use pointer returned by MsgFrameEncode_InitFrame (which will be
 *      buffer + data_offset) to set data in msg.
 *   4. Use MsgFrameEncode_UpdateCRC to populate CRC bytes
 *   5. Use MsgFrameEncode_T::buffer to transmit message.
 * 
 *  Created on: 31 Jul 2022
 *      Author: Liam Flaherty
 */

#ifndef COMM_UART_MSGFRAMEENCODE_H_
#define COMM_UART_MSGFRAMEENCODE_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>

typedef struct {
  uint16_t msgLen; // length of whole message
  uint16_t dataLen; // length of data portion of message
  uint16_t address;
  uint16_t function;
  uint8_t* buffer; // Pointer to full message. Must be of length msgLen
                   // Ownership and management is retained by caller and
                   // borrowed during call.

  CRC_HandleTypeDef* hcrc; // CRC calculation hardware
} MsgFrameEncode_T;

/**
 * @brief Sets up boiler-plate msg contents.
 *   * Begin and end bytes
 *   * Function/address bytes
 * 
 * @param mf 
 * @return Pointer to data contents in message.
 *         Will be a pointer to within mf->buffer
 */
uint8_t* MsgFrameEncode_InitFrame(MsgFrameEncode_T* mf);

/**
 * @brief Add the CRC bytes to the message.
 * 
 * @param mf message frame encoder struct
 */
void MsgFrameEncode_UpdateCRC(MsgFrameEncode_T* mf);

#endif // COMM_UART_MSGFRAMEENCODE_H_
