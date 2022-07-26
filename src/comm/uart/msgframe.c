/*
 * msgframe.c
 *
 *  Created on: 24 Jul 2022
 *      Author: Liam Flaherty
 */

#include "msgframe.h"

#include <string.h>

bool MsgFrame_Init(MsgFrame_T* mf)
{
  if (mf->msgLen > MSGFRAME_BUFFER_LEN) {
    return false;
  }

  mf->start = 0U;
  mf->end = 0U;
  mf->availableBytes = MSGFRAME_BUFFER_LEN;

  return true;
}

bool MsgFrame_RecvBytes(MsgFrame_T* mf,
                        uint8_t* recvBytes,
                        uint16_t recvNumBytes)
{
  if (recvNumBytes > mf->availableBytes) {
    return false;
  }

  memcpy(mf->data + mf->end, recvBytes, recvNumBytes * sizeof(uint8_t));
  mf->end += recvNumBytes;
  mf->availableBytes -= recvNumBytes;

  return true;
}

bool MsgFrame_RecvMsg(MsgFrame_T* mf, size_t* msgOffset)
{
  uint32_t usedBytes = mf->end - mf->start;

  while (usedBytes >= mf->msgLen) {
    // try to extract the next message
    uint8_t startByte = mf->data[mf->start];
    uint8_t crByte = mf->data[mf->start + mf->msgLen - 2U];
    uint8_t lfByte = mf->data[mf->start + mf->msgLen - 1U];

    if (':' == startByte && '\r' == crByte && '\n' == lfByte) {
      // Do CRC check
      bool crc = true; // TODO - implement this
      if (crc) {
        // Valid message
        *msgOffset = mf->start;

        // update internals to next possible message
        usedBytes -= mf->msgLen;
        mf->start += mf->msgLen;

        return true;
      }
    }

    // no valid message yet
    usedBytes--;
    mf->start++;
  }

  if (0U != mf->start) {
    // no valid messages, and data is offset into the buffer
    // shift data down
    for (uint32_t i = 0U; i < usedBytes; ++i) {
      mf->data[i] = mf->data[i + mf->start];
    }
    mf->availableBytes += mf->start;
    mf->end -= mf->start;
    mf->start = 0U;
  }

  return false;
}