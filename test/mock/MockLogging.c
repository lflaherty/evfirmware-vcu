/*
 * MockLogging.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "MockLogging.h"

#include <stdio.h>
#include <string.h>

char mockLogBuffer[MOCK_LOG_BUFFER_LEN];

Logging_Status_T mockLogPrint(const Logging_T* logData, const char* message, const size_t len)
{
	(void)logData;
	(void)message;
	(void)len;
	return LOGGING_STATUS_OK;
}

Logging_Status_T mockLogPrintS(const Logging_T* logData, const char* message, const size_t bufferLen)
{
	(void)logData;

    printf("%s", message);
    memcpy(mockLogBuffer, message, bufferLen*sizeof(char));

	return LOGGING_STATUS_OK;
}