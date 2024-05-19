/*
 * debugtermlib.h
 *
 * Static helper methods for debug terminal.
 *
 *  Created on: 30 Apr 2023
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_PCINTERFACE_DEBUGTERMLIB_H_
#define DEVICE_PCINTERFACE_DEBUGTERMLIB_H_

#include "pcinterface.h"
#include <stdio.h> // for snprintf
#include <stdint.h>
#include <string.h>

#define DEBUGTERM_MAX_MSG_LEN 256

#define DEBUGTERM_MSG_TRUNCATED "[Message truncated]\n"

/**
 * @brief Debug terminal print to serial method.
 * 
 * @param pcinterface PCInterface struct
 * @param msg \0 terminated msg up to a maximum length of `DEBUGTERM_MAX_MSG_LEN`
 */
static void DebugPrint(PCInterface_T* pcinterface, const char* msg)
{
  _Static_assert(PCINTERFACE_MSG_DEBUGTERM_BUFFERLEN >= DEBUGTERM_MAX_MSG_LEN, "mf buffer must have enough space for largest debug term msg");

  uint16_t textLen = (uint16_t)strnlen(msg, DEBUGTERM_MAX_MSG_LEN);
  uint16_t payloadLen = textLen + 2; // For size characters

  // Setup packet
  if (DEBUGTERM_MAX_MSG_LEN == textLen) {
    textLen = sizeof(DEBUGTERM_MSG_TRUNCATED);
    pcinterface->mfDebugEncode.dataLen = payloadLen;
    pcinterface->mfDebugEncode.msgLen = payloadLen + PCINTERFACE_MSG_PACKET_BYTES;
    uint8_t* msgPayload = MsgFrameEncode_InitFrame(&pcinterface->mfDebugEncode);

    msgPayload[0] = (uint8_t)(textLen & 0xFF);
    msgPayload[1] = (uint8_t)((textLen >> 8) & 0xFF);
    snprintf((char*)(msgPayload + 2), textLen, DEBUGTERM_MSG_TRUNCATED);
  } else {
    pcinterface->mfDebugEncode.dataLen = payloadLen;
    pcinterface->mfDebugEncode.msgLen = payloadLen + PCINTERFACE_MSG_PACKET_BYTES;
    uint8_t* msgPayload = MsgFrameEncode_InitFrame(&pcinterface->mfDebugEncode);

    msgPayload[0] = (uint8_t)(textLen & 0xFF);
    msgPayload[1] = (uint8_t)((textLen >> 8) & 0xFF);
    memcpy(msgPayload + 2, msg, textLen);
  }
  MsgFrameEncode_UpdateCRC(&pcinterface->mfDebugEncode);

  // Duplicate the data on both ports (hardware probing is easier this way)
  UART_SendMessage(pcinterface->uartA, pcinterface->mfDebugEncodeBuffer, pcinterface->mfDebugEncode.msgLen);
  UART_SendMessage(pcinterface->uartB, pcinterface->mfDebugEncodeBuffer, pcinterface->mfDebugEncode.msgLen);
}

#endif // DEVICE_PCINTERFACE_DEBUGTERMLIB_H_
