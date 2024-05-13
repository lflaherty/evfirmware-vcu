/*
 * nmeatypes.h
 *
 * Defines types and converters used by NMEA types
 *
 *  Created on: 30 Oct 2022
 *      Author: Liam Flaherty
 */

#ifndef COMM_UART_NMEATYPES_H_
#define COMM_UART_NMEATYPES_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint16_t millisec;
} NmeaUTCTime_T;

/**
 * @brief Converts a "hhmmss.sss\0" to a NmeaUTCTime_T
 * 
 * @param in Input string from NMEA sentence
 * @param out Assigns variables in this pointer
 * @return true If successful
 * @return false On error
 */
bool NmeaCharToUTCTime(char in[11], NmeaUTCTime_T* out);

/**
 * @brief Convert a single digit hex character to a uint8_t.
 * 
 * @param c Digit char
 * @param i Output uint8_t
 * @return true if converted successfully, otherwise false
 */
bool charToUInt8Dec(char c, uint8_t* i);

/**
 * @brief Convert a single digit decimal character to a uint16_t.
 * 
 * @param c Digit char
 * @param i Output uint16_t
 * @return true if converted successfully, otherwise false
 */
bool charToUInt16Dec(char c, uint16_t* i);

/**
 * @brief Convert a single digit hex character to a uint16_t.
 * 
 * @param c Digit char
 * @param i Output uint16_t
 * @return true if converted successfully, otherwise false
 */
bool charToUInt16Hex(char c, uint16_t* i);

/**
 * @brief Convert a string of digits. Must be positive.
 * 
 * @param str String of digits
 * @param len Length of string
 * @param out Ouput digit
 * @return true 
 * @return false 
 */
bool strToUInt8Dec(char* str, uint16_t len, uint8_t* out);

#endif // COMM_UART_NMEATYPES_H_
