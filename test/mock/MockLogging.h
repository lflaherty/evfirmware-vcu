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

#define MOCK_LOG_BUFFER_LEN 5000

// Mock out logPrint method
#define logPrint mockLogPrint
Logging_Status_T mockLogPrint(const Logging_T* logData, const char* message, const size_t len);

// Mock out logPrintS method
#define logPrintS mockLogPrintS
Logging_Status_T mockLogPrintS(const Logging_T* logData, const char* message, const size_t bufferLen);

/**
 * Clear the stored messages
 */
void mockLogClear(void);

/**
 * Print stored messages to stdout
 */
void mockLogDisplay(void);

/**
 * Return a str pointer to the log
 */
const char* mockLogGet(void);

#endif