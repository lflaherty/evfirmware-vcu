/*
 * TestLogging.c
 *
 *  Created on: 3 Aug 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"

// source code under test
#include "lib/logging/logging.c"

static Logging_T mLog;

TEST_GROUP(LIB_LOGGING);

TEST_SETUP(LIB_LOGGING)
{
    Logging_Status_T status = Log_Init(&mLog);
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, status);
}

TEST_TEAR_DOWN(LIB_LOGGING)
{
    mockClearStreamBufferData();
}

TEST(LIB_LOGGING, TestInit)
{
    // TODO
}

TEST_GROUP_RUNNER(LIB_LOGGING)
{
    RUN_TEST_CASE(LIB_LOGGING, TestInit);
}