/*
 * TestCan.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"

#include "lib/logging/logging.h"
#include "comm/can/can.h"

#include <stdio.h>
 
static Logging_T mLog;

TEST_GROUP(COMM_CAN);

TEST_SETUP(COMM_CAN)
{
    mLog.enableLogToDebug = true;
    mLog.enableLogToLogFile = false;
    mLog.enableLogToSerial = false;
    mockLogClear();
    mockSetHALCANStatus(HAL_OK);
}

TEST_TEAR_DOWN(COMM_CAN)
{
    mockLogClear();
    mockSetHALCANStatus(HAL_OK);
}

TEST(COMM_CAN, TestCanInitOk)
{
    CAN_Status_T status = CAN_Init(&mLog);

    TEST_ASSERT(CAN_STATUS_OK == status);

    const char* expectedLogging = 
        "CAN_Init begin\n"
        "CAN_Init complete\n";
	TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

static void RunAllTests(void)
{
	RUN_TEST_CASE(COMM_CAN, TestCanInitOk);
}

int main(int argc, const char* argv[])
{
	return UnityMain(argc, argv, RunAllTests);
}
