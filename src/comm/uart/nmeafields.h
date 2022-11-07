/*
 * nmeafields.h
 *
 * Defines the supported NMEA messages and fields
 *
 *  Created on: 22 Oct 2022
 *      Author: Liam Flaherty
 */

#ifndef COMM_UART_NMEAFIELDS_H_
#define COMM_UART_NMEAFIELDS_H_

#include <stdint.h>
#include <stdbool.h>

#include "nmeatypes.h"

#define NMEA_TALKERMSGID_LEN 6

// $GPGGA - Global Positioning System Fixed Data. Time, Position and fix related data
#define NMEA_GPGGA_TALKERID "GPGGA"
#define NMEA_GPGGA_PAYLOAD_MIN_LENGTH 
struct NmeaMessageGPGGA
{
  NmeaUTCTime_T utcTime;
  char latitude[11];
  char nsIndicator; // 'N'/'S' north/south
  char longitude[11];
  char ewIndicator; // 'E'/'W' east/west
  uint8_t positionFix; 
  uint8_t nSatellites;
};

// $GPRMC - Recommended Minimum Navigation Information
#define NMEA_GPRMC_TALKERID "GPRMC"
struct NmeaMessageGPRMC
{
  NmeaUTCTime_T utcTime;
  char status; // 'A' = valid, 'V' = data not valid
  char latitude[11];
  char nsIndicator; // 'N'/'S' north/south
  char longitude[11];
  char ewIndicator; // 'E'/'W' east/west
  uint16_t speedOverGround; // knots x100
  int16_t courseOverGround; // degrees true
  char date[7]; // ddmmyy
  int16_t magneticVariation; // degrees
  char mode; // 'A' = autonomous mode, 'D' = differential mode, 'E' estimated mode
};

// $GPVTG - Course and speed information relative to the ground
#define NMEA_GPVTG_TALKERID "GPVTG"
struct NmeaMessageGPVTG
{
  int16_t courseTrue; // measured heading, degrees
  int16_t courseMagnetic; // measured heading, degrees
  uint16_t speedKnots; // speed x100
  uint16_t speedKmh; // speed x100
  char mode; // 'A' = autonomous mode, 'D' = differential mode, 'E' estimated mode
};

typedef enum
{
  NMEA_MSGTYPE_NULL   = 0x00U,
  NMEA_MSGTYPE_GPGGA  = 0x01U,
  NMEA_MSGTYPE_GPRMC  = 0x02U,
  NMEA_MSGTYPE_GPVTG  = 0x03U,
} NmeaMessageType_T;

typedef struct
{
  union {
    struct NmeaMessageGPGGA gpgga;
    struct NmeaMessageGPRMC gprmc;
    struct NmeaMessageGPVTG gpvtg;
  } fields;
  NmeaMessageType_T type;
} NmeaMessageFields_T;

#endif // COMM_UART_NMEAFIELDS_H_
