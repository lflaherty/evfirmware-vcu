/**
 * TestPCDebug.c
 * 
 *  Created on: Aug 2 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"
#include "time/tasktimer/MockTasktimer.h"

// source code under test
#include "device/pcdebug/pcdebug.c"

static Logging_T testLog;
static PCDebug_T mPCDebug;

TEST_GROUP(DEVICE_PCDEBUG);

TEST_SETUP(DEVICE_PCDEBUG)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);
    
    memset(&mPCDebug, 0U, sizeof(PCDebug_T));
    PCDebug_Status_T status = PCDebug_Init(&testLog, &mPCDebug);
    TEST_ASSERT(PCDEBUG_STATUS_OK == status);

    const char* expectedLogging =
        "PCDebug_Init begin\n"
        "PCDebug_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for coming tests
    mockLogClear();
}

TEST_TEAR_DOWN(DEVICE_PCDEBUG)
{
    // mockLogClear();
}

TEST(DEVICE_PCDEBUG, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_PCDEBUG, InitTaskRegisterError)
{
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_ERROR_FULL);

    PCDebug_Status_T status = PCDebug_Init(&testLog, &mPCDebug);
    TEST_ASSERT(PCDEBUG_STATUS_ERROR_INIT == status);

    const char* expectedLogging =
        "PCDebug_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST_GROUP_RUNNER(DEVICE_PCDEBUG)
{
    RUN_TEST_CASE(DEVICE_PCDEBUG, InitOk);
    RUN_TEST_CASE(DEVICE_PCDEBUG, InitTaskRegisterError);
}
