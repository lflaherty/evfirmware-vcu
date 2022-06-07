/**
 * TestFaultManager.c
 * 
 *  Created on: Jun 2 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>
#include <stdio.h>

// Mocks for code under test (replaces stubs)
#include "lib/logging/MockLogging.h"

// source code under test
#include "vehicleLogic/stateManager/faultManager.c"

static Logging_T testLog;
static FaultManager_T mFaultMgr;

TEST_GROUP(VEHICLELOGIC_FAULTMANAGER);

TEST_SETUP(VEHICLELOGIC_FAULTMANAGER)
{
    testLog.enableLogToDebug = true;
    testLog.enableLogToLogFile = false;
    testLog.enableLogToSerial = false;
    mockLogClear();

    FaultManager_Init(&testLog, &mFaultMgr);

    const char* expectedLogging =
        "FaultManager_Init begin\n"
        "FaultManager_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for coming tests
    mockLogClear();
}

TEST_TEAR_DOWN(VEHICLELOGIC_FAULTMANAGER)
{
    mockLogClear();
}

TEST(VEHICLELOGIC_FAULTMANAGER, InitOk)
{
    // Done by TEST_SETUP
}

TEST_GROUP_RUNNER(VEHICLELOGIC_FAULTMANAGER)
{
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, InitOk);
}
