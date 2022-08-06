/*
 * TestUart.c
 * 
 *  Created on: 25 Apr 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

// Mocks for code under test
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"

// source code under test
#include "comm/uart/uart.c"

static Logging_T testLog;

TEST_GROUP(COMM_UART);

TEST_SETUP(COMM_UART)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();

    // TODO

    // clear again for the following test
    mockLogClear();
}

TEST_TEAR_DOWN(COMM_UART)
{
    mockLogClear();
    // TODO
}

TEST(COMM_UART, TestUartInitOk)
{
    // Done by TEST_SETUP
}

TEST_GROUP_RUNNER(COMM_UART)
{
    RUN_TEST_CASE(COMM_UART, TestUartInitOk);
}