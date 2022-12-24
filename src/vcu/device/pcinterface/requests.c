/*
 * requests.c
 *
 * Implements logic for requests and message exchanges
 *
 *  Created on: Dec 2 2022
 *      Author: Liam Flaherty
 */

#include "pcinterface.h"

#include <stdio.h> /* for snprintf */

#define BATCH_RECV_SIZE 8U

#define PAYLOAD_OFFSET 5U
#define HEADER_PADDING 11U /* Number of message bytes that aren't payload */


static void handleMsgTestSdc(
    PCInterface_T* pcinterface,
    uint8_t* payloadBytes,
    uint16_t nBytes)
{
  if (nBytes != PCINTERFACE_MSG_COMMON_DATALEN) {
    return;
  }

  bool sdcAssert = !!payloadBytes[7];
  VehicleControl_SetECUError(pcinterface->control, sdcAssert);

  // Construct log message
  char buf[64] = { 0 };
  snprintf(buf, 64, "Test command executed: set SDC ECU error to %u\n", sdcAssert);
  Log_Print(pcinterface->log, buf);
}

static void handleMsgTestPdm(
    PCInterface_T* pcinterface,
    uint8_t* payloadBytes,
    uint16_t nBytes)
{
  if (nBytes != PCINTERFACE_MSG_COMMON_DATALEN) {
    return;
  }

  bool pdmReqeusts[6] = { 0 };
  for (uint8_t i = 0U; i < 6; ++i) {
    pdmReqeusts[i] = !!payloadBytes[i + 2];
  }

  Log_Print(pcinterface->log, "Test command executed: set PDM states to\n");
  for (uint8_t i = 0U; i < 6; ++i) {
    VehicleControl_SetPowerChannel(pcinterface->control, i, pdmReqeusts[i]);

    char buf[32] = { 0 };
    snprintf(buf, 32, "  PDM channel %u: %u\n", i, pdmReqeusts[i]);
    Log_Print(pcinterface->log, buf);
  }
}

static void handleMessage(
    PCInterface_T* pcinterface, 
    uint8_t* bytes,
    uint16_t nBytes)
{
  if (!pcinterface->controlEnabled) {
    // PCInterface_SetVehicleControl hasn't been called yet
    return;
  }
  if (nBytes != PCINTERFACE_MSG_COMMON_MSGLEN) {
    return;
  }

  uint16_t targetAddr = 0U;
  targetAddr |= (bytes[1] & 0xff) << 8;
  targetAddr |= bytes[2] & 0xff;
  if (targetAddr != PCINTERFACE_MSG_DESTADDR_VCU) {
    return;
  }

  uint16_t function = 0U;
  function |= (bytes[3] & 0xff) << 8;
  function |= bytes[4] & 0xff;

  uint8_t* payload = bytes + PAYLOAD_OFFSET;
  uint16_t payloadLen = nBytes - HEADER_PADDING;

  switch (function) {
    case PCINTERFACE_MSG_TESTCMD_SDC_FUNCTION:
      handleMsgTestSdc(pcinterface, payload, payloadLen);
      break;

    case PCINTERFACE_MSG_TESTCMD_PDM_FUNCTION:
      handleMsgTestPdm(pcinterface, payload, payloadLen);
      break;
  }
}

void PCInterface_HandleRequests(PCInterface_T* pcinterface)
{
  uint8_t recvBytes[BATCH_RECV_SIZE] = { 0 };
  while (!xStreamBufferIsEmpty(pcinterface->recvStreamHandle)) {
    uint16_t nRecv = (uint16_t)xStreamBufferReceive(
        pcinterface->recvStreamHandle,
        &recvBytes,
        BATCH_RECV_SIZE,
        0U); // Don't block

    bool succ = MsgFrameDecode_RecvBytes(
        &pcinterface->mfDecode, 
        recvBytes,
        nRecv);

    if (succ) {
      size_t offset = 0U;
      bool msgDecoded = MsgFrameDecode_RecvMsg(
          &pcinterface->mfDecode,
          &offset);
      if (msgDecoded) {
        // By here, a message with a valid length & CRC has been received
        handleMessage(
            pcinterface,
            pcinterface->mfDecode.data + offset,
            pcinterface->mfDecode.msgLen);
      }
    }
  }
}
