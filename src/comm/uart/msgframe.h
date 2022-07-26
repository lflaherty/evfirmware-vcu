/*
 * msgframe.h
 *
 * Usage:
 *   Important: this is not thread safe. This intended to be used synchronously
 *   by a single thread to process messages in a buffer. Use a stream buffer to
 *   acquire data and feed the MsgFrame.
 * 
 *   Use the uart interface to receive serial bytes, and store them in a
 *   buffer. Use this to extract pointers to messages in the buffer.
 *   1. Call MsgFrame_RecvByte(...) when data comes from the ISR stream buffer
 *   2. Repeatedly call MsgFrame_RecvMsg(...) and process the resulting message
 *      (until it returns false)
 *      MsgFrame_RecvMsg will automatically trim the internal buffer once all
 *      messages have been read out (i.e. when it returns false)
 *
 *  Created on: Jul 23 2021
 *      Author: Liam Flaherty
 */

#ifndef COMM_UART_MSGFRAME_H_
#define COMM_UART_MSGFRAME_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#define MSGFRAME_BUFFER_LEN 1024U

typedef struct {
  // MsgFrame settings
  uint16_t msgLen; // length of a single message

  // Variables
  uint16_t start; // offset into `data` where next set of received data starts
  uint16_t end; // offset into `data` where to place next byte of data
  uint16_t availableBytes; // remaining bytes in buffer
  uint8_t data[MSGFRAME_BUFFER_LEN];
} MsgFrame_T;

/**
 * @brief Initialize MsgFrame_T struct
 * 
 * @param msg 
 * @return Success
 */
bool MsgFrame_Init(MsgFrame_T* mf);

/**
 * @brief 
 * @param msg 
 * @param recvBytes 
 * @param recvNumBytes 
 * @return true If all data was inserted.
 */
bool MsgFrame_RecvBytes(MsgFrame_T* mf,
                        uint8_t* recvBytes,
                        uint16_t recvNumBytes);

/**
 * @brief Processes the internal buffer to find the next valid message.
 * If a message was found, *msgOffset will be updated to contain the
 * offset into mf->data. If MsgFrame_RecvMsg returns true, mf->data[msgOffset]
 * will have mf->msgLen bytes of data.
 * 
 * Keep calling this until it returns false.
 * Upon a call that returns false, it will clean the internal buffer to save
 * space for the next set of data to be received.
 * 
 * @param mf MsgFrame struct 
 * @param msgOffset Updates a reference to the buffer offset.
 * @return true A message was found and the value of *msgOffset is valid.
 * @return false No more messages. Do not use the msgOffset result.
 */
bool MsgFrame_RecvMsg(MsgFrame_T* mf, size_t* msgOffset);

#endif // COMM_UART_MSGFRAME_H_