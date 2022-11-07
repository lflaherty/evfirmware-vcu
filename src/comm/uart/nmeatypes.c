/*
 * nmeatypes.c
 *
 *  Created on: 30 Oct 2022
 *      Author: Liam Flaherty
 */

#include "nmeatypes.h"

bool charToUInt8Dec(char c, uint8_t* out)
{
    if (c >= '0' && c <= '9') {
      *out = (unsigned char)c - '0';
      return true;
    }

    return false;
}

bool charToUInt16Dec(char c, uint16_t* out)
{
    if (c >= '0' && c <= '9') {
      *out = (unsigned char)c - '0';
      return true;
    }

    return false;
}

bool charToUInt16Hex(char c, uint16_t* out)
{
    if (c >= '0' && c <= '9') {
      *out = (unsigned char)c - '0';
      return true;
    }
    if (c >= 'A' && c <= 'F') {
      *out = (unsigned char)c - 'A' + 10;
      return true;
    }
    if (c >= 'a' && c <= 'f') {
      *out = (unsigned char)c - 'a' + 10;
      return true;
    }

    return false;
}

bool strToUInt8Dec(char* str, uint16_t len, uint8_t* out)
{
  uint16_t mul = 1;
  *out = 0U;
  for (uint16_t i = 0; i < len; ++i) {
    uint16_t idx = len - i - 1;
    uint16_t digit;
    if (!charToUInt16Dec(str[idx], &digit)) {
      return false;
    }

    *out += mul * digit;
    mul *= 10U;
  }

  return true;
}

bool NmeaCharToUTCTime(char in[11], NmeaUTCTime_T* out)
{
  if (in[10] != '\0') {
    return false;
  }

  uint8_t h1, h0;
  uint8_t m1, m0;
  uint8_t s1, s0;

  uint16_t ms2 = 0U;
  uint16_t ms1 = 0U;
  uint16_t ms0 = 0U;

  if (!charToUInt8Dec(in[0], &h1)) { return false; }
  if (!charToUInt8Dec(in[1], &h0)) { return false; }
  if (!charToUInt8Dec(in[2], &m1)) { return false; }
  if (!charToUInt8Dec(in[3], &m0)) { return false; }
  if (!charToUInt8Dec(in[4], &s1)) { return false; }
  if (!charToUInt8Dec(in[5], &s0)) { return false; }
  if (in[6] == '.') {
    if (!charToUInt16Dec(in[7], &ms2)) { return false; }
    if (!charToUInt16Dec(in[8], &ms1)) { return false; }
    if (!charToUInt16Dec(in[9], &ms0)) { return false; }
  } else if (in[6] != '\0') {
    // This can only be a . or an end of string
    return false;
  }

  out->hour = 10*h1 + h0;
  out->min = 10*m1 + m0;
  out->sec = 10*s1 + s0;
  out->millisec = 100*ms2 + 10*ms1 + ms0;

  return true;
}
