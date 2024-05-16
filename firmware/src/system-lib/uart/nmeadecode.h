/*
 * nmeadecode.h
 *
 * Decoder for GPS serial messages
 * 
 *  Created on: 22 Oct 2022
 *      Author: Liam Flaherty
 */

#ifndef COMM_UART_NMEADECODE_H_
#define COMM_UART_NMEADECODE_H_

#include <stdint.h>
#include <stdbool.h>

#include "crc/crc.h" // TODO check if needed

#include "nmeafields.h"

#define NMEAMSG_BUFFER_LEN 1024U

typedef struct
{
  /**** Internal use ****/
  uint16_t start; // offset into `data` where next set of received data starts
  uint16_t end; // offset into `data` where to place next byte of data
  uint16_t availableBytes; // remaining bytes in buffer
  uint8_t data[NMEAMSG_BUFFER_LEN];
} NmeaDecode_T;

/**
 * @brief Initialize NmeaDecode_T struct
 * 
 * @param nmea 
 * @return Success
 */
void NmeaDecode_Init(NmeaDecode_T* nmea);

/**
 * @brief Copy bytes into internal buffer
 * 
 * @param nmea 
 * @param recvBytes 
 * @param recvNumBytes 
 * @return true If all data was inserted.
 * @return false If not enough room for data, and data not inserted.
 */
bool NmeaDecode_AccumulateBytes(
    NmeaDecode_T* nmea,
    uint8_t* recvBytes,
    uint16_t recvNumBytes);

/**
 * @brief Process the internal buffer to find the next valid message.
 * If a message was found, *outputFields will be update to contain the latest
 * field data.
 * 
 * Keep calling this until it returns false.
 * Upon a call that returns false, it will clean the internal buffer to save
 * space for the next set of data to be received, and outputFields->type will
 * be set to NMEA_MSGTYPE_NULL.
 * 
 * @param nmea NmeaDecode_T struct
 * @param outputFields Output data
 * @return true A message was found and the value of *outputFields has been updated.
 * @return false No more messages.
 */
bool NmeaDecode_Decode(
    NmeaDecode_T* nmea,
    NmeaMessageFields_T* outputFields);

#endif // COMM_UART_NMEADECODE_H_
