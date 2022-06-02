/**
 * TestVehicleStateManager.c
 * 
 *  Created on: May 11 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>
#include <stdio.h>

// Mocks for code under test (replaces stubs)

#include "lib/logging/MockLogging.h"

// source code under test
#include "vehicleLogic/stateManager/vehicleStateManager.c"

static Logging_T testLog;
static VehicleStateManager_T mStateMgr;

TEST_GROUP(VEHICLELOGIC_VEHICLESTATEMANAGER);

TEST_SETUP(VEHICLELOGIC_VEHICLESTATEMANAGER)
{
    testLog.enableLogToDebug = true;
    testLog.enableLogToLogFile = false;
    testLog.enableLogToSerial = false;
    mockLogClear();

    memset(&mStateMgr, 0, sizeof(VehicleStateManager_T));

    VehicleStateManager_Status_T status = VehicleStateManager_Init(&testLog, &mStateMgr);
    TEST_ASSERT(STATEMANAGER_STATUS_OK == status);

    const char* expectedLogging =
        "VehicleStateManager_Init begin\n"
        "VehicleStateManager_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for coming tests
    mockLogClear();
}

TEST_TEAR_DOWN(VEHICLELOGIC_VEHICLESTATEMANAGER)
{
    mockLogClear();
}

TEST(VEHICLELOGIC_VEHICLESTATEMANAGER, InitOk)
{
    // Done by TEST_SETUP
}

TEST_GROUP_RUNNER(VEHICLELOGIC_VEHICLESTATEMANAGER)
{
    RUN_TEST_CASE(VEHICLELOGIC_VEHICLESTATEMANAGER, InitOk);
}
