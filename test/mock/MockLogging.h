/*
 * MockLogging.h
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

// Mock must be defined first in a test case
#ifdef LIB_LOGGING_LOGGING_H_
#error "Header already defined - cannot mock"
#else
#define LIB_LOGGING_LOGGING_H_

#define LIB_LOGGING_LOGGING_H_

#include <stdio.h>

#define MOCK_LOG_BUFFER_LEN 256
extern char mockLogBuffer[MOCK_LOG_BUFFER_LEN]; // stores the last entry that was printed

typedef enum
{
	LOGGING_STATUS_OK = 0x00U,
	LOGGING_STATUS_LOG_ERROR = 0x01U
} Logging_Status_T;

typedef struct {
} Logging_T; // Not used in mock

Logging_Status_T logPrint(const Logging_T* logData, const char* message, const size_t len);

Logging_Status_T logPrintS(const Logging_T* logData, const char* message, const size_t bufferLen);

#endif