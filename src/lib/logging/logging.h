/*
 * logging.h
 *
 *  Created on: 24 Jul 2021
 *      Author: Liam Flaherty
 */

#ifndef LIB_LOGGING_LOGGING_H_
#define LIB_LOGGING_LOGGING_H_

#include "stm32f7xx_hal.h"
#include <stddef.h>
#include <stdbool.h>

#define LOGGING_DEFAULT_BUFF_LEN 256

/**
 * These error statuses are arranged to be bit fields.
 */
typedef enum
{
  LOGGING_STATUS_OK         = 0x00U,
  LOGGING_STATUS_LOG_ERROR  = 0x01U
} Logging_Status_T;

typedef struct {
  bool enableLogToSerial;
  bool enableLogToDebug;
  bool enableLogToLogFile;

  UART_HandleTypeDef* handleSerial; // Serial port to log to

  // TODO log file
} Logging_T;

/**
 * Log a message
 * @param logData Log config
 * @param message Log text
 * @param len Length of message
 * @return Success/fail status
 */
Logging_Status_T logPrint(const Logging_T* logData, const char* message, const size_t len);

/**
 * Print a string, and determine it's size (up to bufferLen)
 * @param logData Log config
 * @param message Log text
 * @param bufferLen Length of buffer that stores the string
 * @return Success/fail status
 */
Logging_Status_T logPrintS(const Logging_T* logData, const char* message, const size_t bufferLen);

/**
 * strlen with a max iteration length
 */
size_t strnlen(const char* str, const size_t maxBufferLen);

#endif /* LIB_LOGGING_LOGGING_H_ */
