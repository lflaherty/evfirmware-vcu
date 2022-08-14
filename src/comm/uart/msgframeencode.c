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
  assert(NULL != mf->hcrc);
  assert(CRC_INPUTDATA_FORMAT_BYTES == mf->hcrc->InputDataFormat);

  size_t crcOffset = mf->msgLen - 6U;
  memset(mf->buffer + crcOffset, 0U, sizeof(uint32_t));

  uint32_t calcCrc = HAL_CRC_Calculate(mf->hcrc, (void*)mf->buffer, mf->msgLen);
  memcpy(mf->buffer + crcOffset, &calcCrc, sizeof(uint32_t));
}