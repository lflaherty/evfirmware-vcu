/*
 * nmeadecode.c
 *
 *  Created on: 22 Oct 2022
 *      Author: Liam Flaherty
 */

#include "nmeadecode.h"
#include "nmeatypes.h"
#include "nmeafields.h"

#include <stdio.h>
#include <string.h>

/*
 * Minimum derived from a minimal viable message being:
 * $XYABC,D*EF<CR><LF>
 * Where:
 *  XY    Talker
 *  ABC   Message ID
 *  D     Data of at least length 1
 *  *EF   Checksum
 */
#define MIN_MSG_LEN 13
#define START_DELIMITER '$'
#define FIELD_DELIMITER ','
#define CHECKSUM_DELIMITER '*'

// Used by decoder to pass around
struct decodeInfo {
  NmeaDecode_T* nmea;

  // Used for working through message
  uint16_t i;
  bool fullMsgReceived;
  uint16_t msgLen;

  // Useful decode info
  uint16_t msgBegin; // index of '$' in nmea->data
  char talkermsgId[NMEA_TALKERMSGID_LEN]; // combine talker and message ID into one (e.g. GPGGA)
  uint16_t payloadBegin;
  uint16_t payloadEnd; // index of '*' (next element after payload)
  char checksum[3]; // extra char for null terminator
};

// Used to decode payload tokens
struct token {
  char* inStr; // pointer to input string array
  uint16_t inStrLen; // length left of data
  uint16_t i; // pointer to start of string in array
  char tokenBuffer[128]; // tokens are stored here
};

#define MIN_LEN_TOKEN_REQUIRED 2 /* minimum token length for a required token */
#define MIN_LEN_TOKEN_OPTIONAL 1 /* minimum token length for an optional*/
// Macros to be used exclusively in payload decoder methods.
#define TOKEN_REQUIRED(tokenLen) if ((tokenLen) < MIN_LEN_TOKEN_REQUIRED) { return false; }
#define TOKEN_OPTIONAL(tokenLen) if ((tokenLen) < MIN_LEN_TOKEN_OPTIONAL) { return false; }

// ------------------------- Private methods -------------------------
// ************************* Message payload decoding *************************
/**
 * @brief Finds the next token in a the payload string (separated by ,)
 * 
 * @param token Pointer to token struct for intermediate results
 * @return uint16_t Number of characters in token. 0 if no token. Note that
 * this includes the null terminator. A length of 1 indicates empty.
 */
static inline uint16_t extractNextToken(struct token* token)
{
  if (token->inStrLen == 0) {
    token->tokenBuffer[0] = '\0';
    return 0;
  }

  uint16_t currentCount = 0U; // output index counter
  char c = token->inStr[token->i];

  while (c != ',' && token->i < token->inStrLen) {
    token->tokenBuffer[currentCount] = c;

    currentCount++;
    token->i++;
    c = token->inStr[token->i];
  }

  token->tokenBuffer[currentCount] = '\0';

  // The loop skips the ',' so skip over that
  currentCount++;
  token->i++;

  return currentCount;
}

static inline bool payloadDecodeGPGGA(struct decodeInfo* dec, struct NmeaMessageGPGGA* msgData)
{
  struct token token = { 0 };
  token.inStr = (char*)(dec->nmea->data + dec->payloadBegin);
  token.inStrLen = dec->payloadEnd - dec->payloadBegin;

  uint16_t count;

  // Field: UTC time
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  if (!NmeaCharToUTCTime(token.tokenBuffer, &msgData->utcTime)) {
    return false;
  }

  // Field: Latitude
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  if (count > 11) {
    // More characters than expected...
    return false;
  }
  strncpy(msgData->latitude, token.tokenBuffer, count * sizeof(char));

  // Field: N/S
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  char fieldNS = token.tokenBuffer[0];
  if (fieldNS != 'N' && fieldNS != 'S') {
    return false;
  }
  msgData->nsIndicator = fieldNS;

  // Field: Longitude
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  if (count > 11) {
    // More characters than expected...
    return false;
  }
  strncpy(msgData->longitude, token.tokenBuffer, count * sizeof(char));

  // Field: E/W
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  char fieldEW = token.tokenBuffer[0];
  if (fieldEW != 'E' && fieldEW != 'W') {
    return false;
  }
  msgData->ewIndicator = fieldEW;

  // Field: Position fix indicator
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  if (!charToUInt8Dec(token.tokenBuffer[0], &msgData->positionFix)) {
    return false;
  }

  // Field: Num satellites
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  if (!strToUInt8Dec(token.tokenBuffer, count - 1, &msgData->nSatellites)) {
    return false;
  }

  // Field: HDOP (Horizontal diulation of precision)
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  // this field is unused

  // Field: MSL Altitude
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  // this field is unused

  // Field: MSL Altitude units
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  // this field is unused

  // Field: Geoidal separation
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  // this field is unused

  // Field: Geoidal separation units
  count = extractNextToken(&token);
  TOKEN_REQUIRED(count);
  // this field is unused

  // Field: Age of diff coordination
  count = extractNextToken(&token);
  TOKEN_OPTIONAL(count);
  // this field is unused

  return true;
}

/**
 * @brief Attempts decode the payload of the current message.
 * If ok, update outputFields, if not, outputFields->type is set to NMEA_MSGTYPE_NULL.
 * 
 * @param nmea 
 * @return true if successful, false if the message contains errors
 */
static inline bool tryDecodePayload(struct decodeInfo* dec, NmeaMessageFields_T* outputFields)
{
  bool payloadDecoded = false;
  if (strncmp(dec->talkermsgId, NMEA_GPGGA_TALKERID, NMEA_TALKERMSGID_LEN) == 0) {
    outputFields->type = NMEA_MSGTYPE_GPGGA;
    payloadDecoded = payloadDecodeGPGGA(dec, &outputFields->fields.gpgga);

  // TODO not implemented
  // } else if (strncmp(dec->talkermsgId, NMEA_GPRMC_TALKERID, NMEA_TALKERMSGID_LEN) == 0) {
  //   printf("Received GPRMC\n");
  //   outputFields->type = NMEA_MSGTYPE_GPRMC;
  // } else if (strncmp(dec->talkermsgId, NMEA_GPVTG_TALKERID, NMEA_TALKERMSGID_LEN) == 0) {
  //   printf("Received GPVTG\n");
  //   outputFields->type = NMEA_MSGTYPE_GPVTG;

  } else {
    outputFields->type = NMEA_MSGTYPE_NULL;
  }

  return payloadDecoded;
}

// ************************* Generic message decoding *************************
static inline bool verifyChecksum(struct decodeInfo* dec)
{
  if (dec->payloadEnd < (dec->payloadBegin + 2U)) {
    return false;
  }

  if (dec->checksum[2] != '\0') {
    return false;
  }

  uint16_t c1, c0;
  if (!charToUInt16Hex(dec->checksum[0], &c1)) {
    return false;
  }
  if (!charToUInt16Hex(dec->checksum[1], &c0)) {
    return false;
  }
  uint16_t checksumMsg = 16*c1 + c0;

  // Checksum is the XOR of all bytes between $ and * (not including)
  uint16_t checksumCalc = 0U;
  for (uint16_t j = dec->msgBegin + 1; j < dec->payloadEnd; ++j) {
    checksumCalc ^= dec->nmea->data[j];
  }

  return checksumCalc == checksumMsg;
}

/**
 * @brief Trim all characters from the front that aren't START_DELIMITER
 * No valid message can begin if these are there.
 * 
 * @param nmea 
 */
static inline void trimFront(NmeaDecode_T* nmea)
{
  uint16_t i = nmea->start;
  while (i < nmea->end && nmea->data[i] != START_DELIMITER) {
    nmea->start++;
    i++;
  }
}

/**
 * @brief Locate the initial '$' or return false
 * 
 * @param dec 
 * @return true 
 * @return false 
 */
static inline bool tryDecodeStart(struct decodeInfo* dec)
{
  if (START_DELIMITER != dec->nmea->data[dec->i]) {
    return false;
  }
  dec->msgBegin = dec->i;

  dec->i++;

  return true;
}

/**
 * @brief Locate and fetch the talker/msg id or return false.
 * 
 * @param dec 
 * @return true 
 * @return false 
 */
static inline bool tryDecodeTalkerMsgId(struct decodeInfo* dec)
{
  memcpy(dec->talkermsgId, dec->nmea->data + dec->i, 5U);
  dec->i += 5U;

  // ID/payload delimiter
  if (FIELD_DELIMITER != dec->nmea->data[dec->i]) {
    return false;
  }
  dec->i++;

  return true;
}

/**
 * @brief Locate the start and end of payload or return false.
 * 
 * @param dec 
 * @return true 
 * @return false 
 */
static inline bool tryDecodeLocatePayload(struct decodeInfo* dec)
{
  dec->payloadBegin = dec->i;
  bool checksumFound = false;
  while (dec->i <= dec->nmea->end) {
    if (dec->nmea->data[dec->i] == CHECKSUM_DELIMITER) {
      dec->payloadEnd = dec->i;
      checksumFound = true;
      break;
    }
    dec->i++;
  }
  dec->i++; // loop doesn't increment when it hits the '*'

  if (!checksumFound) {
    return false;
  }

  return true;
}

/**
 * @brief Locate the checksum bytes and message termination or return false
 * 
 * @param dec 
 * @return true 
 * @return false 
 */
static inline bool tryDecodeLocateMsgEnd(struct decodeInfo* dec)
{
  // Now we require another 4 characters after this
  uint16_t remainingBytes = dec->nmea->end - dec->i;
  if (remainingBytes < 4) {
    return false;
  }

  dec->checksum[0] = (char)dec->nmea->data[dec->i++];
  dec->checksum[1] = (char)dec->nmea->data[dec->i++];

  // termination
  if (dec->nmea->data[dec->i++] != '\r') {
    return false;
  }
  if (dec->nmea->data[dec->i++] != '\n') {
    return false;
  }

  return true;
}

/**
 * @brief Attempts decode on the current content of the buffer.
 * If ok, update outputFields, if not, outputFields->type is set to NMEA_MSGTYPE_NULL.
 * 
 * @param nmea 
 * @return length of sentence if valid sentence was found, otherwise 0
 */
static inline uint16_t tryDecode(NmeaDecode_T* nmea, NmeaMessageFields_T* outputFields)
{
  outputFields->type = NMEA_MSGTYPE_NULL;

  if (nmea->end == nmea->start) {
    // Need at least some data
    return 0;
  }

  struct decodeInfo decodeInfo = { 0 };
  decodeInfo.nmea = nmea;

  // Guarantee that the required data is available for a message
  decodeInfo.i = nmea->start;
  uint16_t remainingBytes = nmea->end - decodeInfo.i;

  if (remainingBytes < MIN_MSG_LEN) {
    // Not enough bytes to make up minimum message
    return 0;
  }

  // Fetch the message components
  bool messageDecodeSteps = 
      tryDecodeStart(&decodeInfo) &&
      tryDecodeTalkerMsgId(&decodeInfo) &&
      tryDecodeLocatePayload(&decodeInfo) &&
      tryDecodeLocateMsgEnd(&decodeInfo);
  if (!messageDecodeSteps) {
    return 0;
  }

  // Check checksum
  if (!verifyChecksum(&decodeInfo)) {
    return 0;
  }

  // Process payload
  decodeInfo.msgLen = decodeInfo.i;
  decodeInfo.fullMsgReceived = true;
  if (!tryDecodePayload(&decodeInfo, outputFields)) {
    return 0;
  }

  return decodeInfo.msgLen;
}


// ------------------------- Public methods -------------------------
void NmeaDecode_Init(NmeaDecode_T* nmea)
{
  nmea->start = 0U;
  nmea->end = 0U;
  nmea->availableBytes = NMEAMSG_BUFFER_LEN;
}

bool NmeaDecode_AccumulateBytes(
    NmeaDecode_T* nmea,
    uint8_t* recvBytes,
    uint16_t recvNumBytes)
{
  if (recvNumBytes > nmea->availableBytes) {
    return false;
  }

  memcpy(nmea->data + nmea->end, recvBytes, recvNumBytes * sizeof(uint8_t));
  nmea->end += recvNumBytes;
  nmea->availableBytes -= recvNumBytes;

  return true;
}

bool NmeaDecode_Decode(
    NmeaDecode_T* nmea,
    NmeaMessageFields_T* outputFields)
{
  // make sure we're starting with something useful in the buffer
  trimFront(nmea);

  uint16_t usedBytes = nmea->end - nmea->start;

  while (usedBytes > MIN_MSG_LEN) {
    uint16_t msgLen = tryDecode(nmea, outputFields);
    if (0U == msgLen) {
      // no valid message yet, increment by one character
      usedBytes--;
      nmea->start++;
    } else {
      // Found a whole message, so step to next possible message
      usedBytes -= msgLen;
      nmea->start += msgLen;

      return true;
    }
  }

  if (nmea->start > 1U) {
    // no valid messages, and data is offset into the buffer
    // shift data down
    for (uint32_t i = 0U; i < usedBytes; ++i) {
      nmea->data[i] = nmea->data[i + nmea->start];
    }
    nmea->availableBytes += nmea->start;
    nmea->end -= nmea->start;
    nmea->start = 0U;
  }

  return false;
}
