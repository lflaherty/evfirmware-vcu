/*
 * msgframeencode.c
 *
 *  Created on: 31 Jul 2022
 *      Author: Liam Flaherty
 */

#include "msgframeencode.h"

#include <assert.h>
#include <string.h>

// Number of bytes that make up:
// start byte, end bytes, addr, function, crc
#define MSGFRAME_NUM_FRAME_BYTES 11U

static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms

uint8_t* MsgFrameEncode_InitFrame(MsgFrameEncode_T* mf)
{
  assert(mf->msgLen >= MSGFRAME_NUM_FRAME_BYTES);
  assert(mf->msgLen - MSGFRAME_NUM_FRAME_BYTES == mf->dataLen);

  mf->buffer[0] = ':';
  mf->buffer[mf->msgLen - 2U] = '\r';
  mf->buffer[mf->msgLen - 1U] = '\n';

  // Address[1:0]
  mf->buffer[1] = (mf->address >> 8) & 0xFF;
  mf->buffer[2] = (mf->address >> 0) & 0xFF;

  // Function[1:0]
  mf->buffer[3] = (mf->function >> 8) & 0xFF;
  mf->buffer[4] = (mf->function >> 0) & 0xFF;

  return mf->buffer + 5U;
}

void MsgFrameEncode_UpdateCRC(MsgFrameEncode_T* mf)
{
  // This required for the (void*) cast to work
  assert(NULL != mf->crc);
  assert(NULL != mf->crc->hcrc);
  assert(CRC_INPUTDATA_FORMAT_BYTES == mf->crc->hcrc->InputDataFormat);

  size_t crcOffset = mf->msgLen - 6U;
  mf->buffer[crcOffset + 0U] = 0U;
  mf->buffer[crcOffset + 1U] = 0U;
  mf->buffer[crcOffset + 2U] = 0U;
  mf->buffer[crcOffset + 3U] = 0U;

  uint32_t calcCrc = 0U;
  CRC_Calculate(
    mf->crc,
    (void*)mf->buffer,
    mf->msgLen,
    mBlockTime,
    &calcCrc);

  mf->buffer[crcOffset + 0U] = (calcCrc >> 24U) & 0xFF;
  mf->buffer[crcOffset + 1U] = (calcCrc >> 16U) & 0xFF;
  mf->buffer[crcOffset + 2U] = (calcCrc >> 8U) & 0xFF;
  mf->buffer[crcOffset + 3U] = (calcCrc >> 0U) & 0xFF;
}
