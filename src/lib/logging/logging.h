/*
 * logging.h
 *
 *  Created on: 24 Jul 2021
 *      Author: Liam Flaherty
 */

#ifndef LIB_LOGGING_LOGGING_H_
#define LIB_LOGGING_LOGGING_H_

#include "FreeRTOS.h"
#include "semphr.h"
#include "stream_buffer.h"
#include <stddef.h>
#include <stdbool.h>

#define LOGGING_DEFAULT_BUFF_LEN 256

/**
 * These error statuses are arranged to be bit fields.
 */
typedef enum
{
  LOGGING_STATUS_OK           = 0x00U,
  LOGGING_STATUS_LOG_ERROR    = 0x01U,
  LOGGING_STATUS_ERROR_MUTEX  = 0x02U
} Logging_Status_T;

typedef struct {
  // ******* Internal use *******
  // Mutex lock
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;

  // output modes
  bool enableSerial;
  bool enableSWO; // SWD serial wire output

  StreamBufferHandle_t serialOutputStream;
} Logging_T;

/**
 * @brief Initialize logging module
 *
 * @param log Log struct
 * @return LOGGING_STATUS_OK if successful
 */
Logging_Status_T Log_Init(Logging_T* log);

/**
 * @brief Sets and enables serial stream output.
 * 
 * @param log Log struct
 * @param stream Serial stream. This type is actually a pointer.
 * @return LOGGING_STATUS_OK if successful
 */
Logging_Status_T Log_SetSerialStream(
    Logging_T* log,
    StreamBufferHandle_t stream);

/**
 * @brief Enables SWO output.
 * 
 * @param log Log struct
 * @return LOGGING_STATUS_OK if successful 
 */
Logging_Status_T Log_EnableSWO(Logging_T* log);

/**
 * @brief Prints message to enabled log output streams.
 * msg should be terminated with a '\0' character.
 * 
 * @param log Log struct
 * @param msg Char array of message.
 */
void Log_Print(Logging_T* log, const char* msg);

/**
 * Log a message
 * @param logData Log config
 * @param message Log text
 * @param len Length of message
 * @return Success/fail status
 */
Logging_Status_T logPrint(Logging_T* logData, const char* message, const size_t len);

/**
 * Print a string, and determine it's size (up to bufferLen)
 * @param logData Log config
 * @param message Log text
 * @param bufferLen Length of buffer that stores the string
 * @return Success/fail status
 */
Logging_Status_T logPrintS(Logging_T* logData, const char* message, const size_t bufferLen);

/**
 * strlen with a max iteration length
 */
size_t strnlen(const char* str, const size_t maxBufferLen);

#endif /* LIB_LOGGING_LOGGING_H_ */
