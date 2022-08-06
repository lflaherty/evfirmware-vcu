/*
 * MockLogging.h
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_MOCKLOGGING_H_
#define _MOCK_MOCKLOGGING_H_

#include <stdio.h>

#define Log_Print stubLogPrint

// Bring in header to be mocked 
#include "lib/logging/logging.h"

#define MOCK_LOG_BUFFER_LEN 5000

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