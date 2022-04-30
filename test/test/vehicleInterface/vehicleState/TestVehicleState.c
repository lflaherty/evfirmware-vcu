/**
 * TestVehicleState.c
 * 
 *  Created on: Oct 16 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"

// source code under test
#include "vehicleInterface/vehicleState/vehicleState.c"

static Logging_T testLog;

TEST_GROUP(VEHICLEINTERFACE_VEHICLESTATE);

TEST_SETUP(VEHICLEINTERFACE_VEHICLESTATE)
{
    testLog.enableLogToDebug = true;
    testLog.enableLogToLogFile = false;
    testLog.enableLogToSerial = false;
    mockLogClear();
}

TEST_TEAR_DOWN(VEHICLEINTERFACE_VEHICLESTATE)
{
    mockLogClear();
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, TestVehicleStateInitOk)
{

}

TEST_GROUP_RUNNER(VEHICLEINTERFACE_VEHICLESTATE)
{
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, TestVehicleStateInitOk);
}
