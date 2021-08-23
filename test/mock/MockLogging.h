/*
 * MockLogging.h
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_MOCKLOGGING_H_
#define _MOCK_MOCKLOGGING_H_

#include <stdio.h>

// Bring in header to be mocked 
#include "lib/logging/logging.h"

// Data for mock interface
#define MOCK_LOG_BUFFER_LEN 256
extern char mockLogBuffer[MOCK_LOG_BUFFER_LEN]; // stores the last entry that was printed

// Mock out logPrint method
#define logPrint mockLogPrint
Logging_Status_T mockLogPrint(const Logging_T* logData, const char* message, const size_t len);

// Mock out logPrintS method
#define logPrintS mockLogPrintS
Logging_Status_T mockLogPrintS(const Logging_T* logData, const char* message, const size_t bufferLen);

#endif