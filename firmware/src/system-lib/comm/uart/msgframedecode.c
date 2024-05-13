/*
 * msgframedecode.c
 *
 *  Created on: 24 Jul 2022
 *      Author: Liam Flaherty
 */

#include "msgframedecode.h"

#include <string.h>
#include <assert.h>

static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms

// ------------------- Private methods -------------------
static bool verifyCrc(MsgFrameDecode_T* mf)
{
  // This required for the (void*) cast to work
  assert(NULL != mf->crc);
  assert(NULL != mf->crc->hcrc);
  assert(CRC_INPUTDATA_FORMAT_BYTES == mf->crc->hcrc->InputDataFormat);

  uint8_t* msg = mf->data + mf->start;
  size_t crcOffset = mf->msgLen - 6U;

  uint32_t msgCrc = 0U;
  msgCrc |= (uint32_t)msg[crcOffset + 0U] << 24U;
  msgCrc |= (uint32_t)msg[crcOffset + 1U] << 16U;
  msgCrc |= (uint32_t)msg[crcOffset + 2U] << 8U;
  msgCrc |= (uint32_t)msg[crcOffset + 3U] << 0U;
  memset(msg + crcOffset, 0U, sizeof(uint32_t));

  uint32_t calcCrc = 0U;
  CRC_Calculate(
    mf->crc,
    (void*)msg,
    mf->msgLen,
    mBlockTime,
    &calcCrc);

  // restore original message
  msg[crcOffset + 0U] = (msgCrc >> 24U) & 0xFF;
  msg[crcOffset + 1U] = (msgCrc >> 16U) & 0xFF;
  msg[crcOffset + 2U] = (msgCrc >> 8U) & 0xFF;
  msg[crcOffset + 3U] = (msgCrc >> 0U) & 0xFF;

  return calcCrc == msgCrc;
}

// ------------------- Public methods -------------------
bool MsgFrameDecode_Init(MsgFrameDecode_T* mf)
{
  if (mf->msgLen > MSGFRAME_BUFFER_LEN) {
    return false;
  }

  mf->start = 0U;
  mf->end = 0U;
  mf->availableBytes = MSGFRAME_BUFFER_LEN;

  return true;
}

bool MsgFrameDecode_RecvBytes(MsgFrameDecode_T* mf,
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

bool MsgFrameDecode_RecvMsg(MsgFrameDecode_T* mf, size_t* msgOffset)
{
  uint32_t usedBytes = mf->end - mf->start;

  while (usedBytes >= mf->msgLen) {
    // try to extract the next message
    uint8_t startByte = mf->data[mf->start];
    uint8_t crByte = mf->data[mf->start + mf->msgLen - 2U];
    uint8_t lfByte = mf->data[mf->start + mf->msgLen - 1U];

    if (':' == startByte && '\r' == crByte && '\n' == lfByte) {
      // Do CRC check
      if (verifyCrc(mf)) {
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

  if (mf->start > 1U) {
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
