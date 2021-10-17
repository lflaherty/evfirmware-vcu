/**
 * TestVehicleState.c
 * 
 *  Created on: Oct 16 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"

#include "vehicleInterface/vehicleState/vehicleState.h"

static Logging_T mLog;

TEST_GROUP(VEHICLEINTERFACE_VEHICLESTATE);

TEST_SETUP(VEHICLEINTERFACE_VEHICLESTATE)
{
    mLog.enableLogToDebug = true;
    mLog.enableLogToLogFile = false;
    mLog.enableLogToSerial = false;
    mockLogClear();
}

TEST_TEAR_DOWN(VEHICLEINTERFACE_VEHICLESTATE)
{
    mockLogClear();
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, TestVehicleStateInitOk)
{

}

static void RunAllTests(void)
{
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, TestVehicleStateInitOk);
}

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, RunAllTests);
}
