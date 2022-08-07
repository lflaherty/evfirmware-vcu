/*
 * TestLogging.c
 *
 *  Created on: 3 Aug 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <string.h>

// This code must go before other includes
#define PRINTF_BUFFER_LEN 257
static char printfOut[PRINTF_BUFFER_LEN];
static int printfOutSize = 0;

#define printf(format, args...) \
    printfOutSize += snprintf(printfOut + printfOutSize, PRINTF_BUFFER_LEN, format, ## args);

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

// source code under test
#include "lib/logging/logging.c"

static Logging_T mLog;
static StreamBufferHandle_t mSerialStream;

#define STREAM_BUFFER_MAX_LEN 512

// Helper methods
void printfBufClear(void)
{
    printfOutSize = 0U;
    memset(printfOut, 0U, sizeof(printfOut) / sizeof(char));
}

TEST_GROUP(LIB_LOGGING);

TEST_SETUP(LIB_LOGGING)
{
    printfBufClear();
    mockClearStreamBufferData();
    mockSemaphoreSetLocked(false);

    Logging_Status_T status = Log_Init(&mLog);
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, status);
}

TEST_TEAR_DOWN(LIB_LOGGING)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());
}

TEST(LIB_LOGGING, TestInit)
{
    // Done in setup
}

TEST(LIB_LOGGING, TestNoLog)
{
    // No output streams
    Log_Print(&mLog, "Log message\n");
    TEST_ASSERT_EQUAL(0U, printfOutSize);
    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen());
}

TEST(LIB_LOGGING, TestLogSWO)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&mLog));

    // Only SWO
    char sampleMsg[] = "Log message\n";
    size_t sampleMsgLen = sizeof(sampleMsg) - 1U; // logging shouldn't send the '\0' char

    Log_Print(&mLog, "Log message\n");

    TEST_ASSERT_EQUAL(sampleMsgLen, printfOutSize);
    TEST_ASSERT_EQUAL_STRING(sampleMsg, printfOut);
    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen());
}

TEST(LIB_LOGGING, TestLogSerial)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK,
                      Log_SetSerialStream(&mLog, mSerialStream));

    // Only serial stream
    char sampleMsg[] = "Log message\n";
    size_t sampleMsgLen = sizeof(sampleMsg) - 1U; // logging shouldn't send the '\0' char

    Log_Print(&mLog, "Log message\n");

    TEST_ASSERT_EQUAL(0U, printfOutSize);
    TEST_ASSERT_EQUAL(sampleMsgLen, mockGetStreamBufferLen());

    char streamBufferData[STREAM_BUFFER_MAX_LEN] = {0};
    mockGetStreamBufferData((char*)streamBufferData, STREAM_BUFFER_MAX_LEN);
    TEST_ASSERT_EQUAL_STRING(sampleMsg, streamBufferData);
}

TEST(LIB_LOGGING, TestEnableSWOBusy)
{
    mockSemaphoreSetLocked(true);
    TEST_ASSERT_EQUAL(LOGGING_STATUS_ERROR_MUTEX, Log_EnableSWO(&mLog));

    mockSemaphoreSetLocked(false);
}

TEST(LIB_LOGGING, TestSetSerialStreamBusy)
{
    mockSemaphoreSetLocked(true);
    TEST_ASSERT_EQUAL(LOGGING_STATUS_ERROR_MUTEX,
                      Log_SetSerialStream(&mLog, mSerialStream));

    mockSemaphoreSetLocked(false);
}

TEST(LIB_LOGGING, TestLogBusy)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&mLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK,
                      Log_SetSerialStream(&mLog, mSerialStream));

    mockSemaphoreSetLocked(true);

    // No data outputs, despite output enabled
    Log_Print(&mLog, "Log message\n");
    TEST_ASSERT_EQUAL(0U, printfOutSize);
    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen());

    mockSemaphoreSetLocked(false);
}

TEST(LIB_LOGGING, TestLogLongMessages)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&mLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK,
                      Log_SetSerialStream(&mLog, mSerialStream));

    // To protect against missing \0 characters, the output will truncate

    // create 500 'X' characters
    char longMessage[500] = {0};
    for (size_t i = 0; i < 500; ++i) {
        longMessage[i] = 'X';
    }

    Log_Print(&mLog, longMessage);

    // SWO
    char expectedPrintf[] = "[Skipping truncated message on SWO]\n";
    size_t expectedPrintfLen = sizeof(expectedPrintf) - 1U;
    TEST_ASSERT_EQUAL(expectedPrintfLen, printfOutSize);
    TEST_ASSERT_EQUAL_STRING(expectedPrintf, printfOut);

    // Serial
    TEST_ASSERT_EQUAL(256, mockGetStreamBufferLen());
    char streamBufferData[STREAM_BUFFER_MAX_LEN] = {0};
    mockGetStreamBufferData((char*)streamBufferData, STREAM_BUFFER_MAX_LEN);
    TEST_ASSERT_EQUAL_CHAR_ARRAY(longMessage, streamBufferData, 256);
}

TEST_GROUP_RUNNER(LIB_LOGGING)
{
    RUN_TEST_CASE(LIB_LOGGING, TestInit);
    RUN_TEST_CASE(LIB_LOGGING, TestNoLog);
    RUN_TEST_CASE(LIB_LOGGING, TestLogSWO);
    RUN_TEST_CASE(LIB_LOGGING, TestLogSerial);
    RUN_TEST_CASE(LIB_LOGGING, TestEnableSWOBusy);
    RUN_TEST_CASE(LIB_LOGGING, TestSetSerialStreamBusy);
    RUN_TEST_CASE(LIB_LOGGING, TestLogBusy);
    RUN_TEST_CASE(LIB_LOGGING, TestLogLongMessages);
}