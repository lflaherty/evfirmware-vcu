/**
 * TestVehicleState.c
 * 
 *  Created on: Oct 16 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>
#include <stdio.h>

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"
#include "time/tasktimer/MockTasktimer.h"

// source code under test
#include "vehicleInterface/vehicleState/vehicleState.c"

static Logging_T testLog;
static VehicleState_T mState;

TEST_GROUP(VEHICLEINTERFACE_VEHICLESTATE);

TEST_SETUP(VEHICLEINTERFACE_VEHICLESTATE)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);
    
    memset(&mState, 0, sizeof(VehicleState_T));

    VehicleState_Status_T status = VehicleState_Init(&testLog, &mState);
    TEST_ASSERT(VEHICLESTATE_STATUS_OK == status);

    const char* expectedLogging =
        "VehicleState_Init begin\n"
        "VehicleState_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for coming tests
    mockLogClear();
}

TEST_TEAR_DOWN(VEHICLEINTERFACE_VEHICLESTATE)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());
    mockLogClear();
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, InitOk)
{
    // Done by TEST_SETUP
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, CopyState)
{
    mState.data.motor.calculatedTorque = 420.0f;
    mState.data.motor.phaseACurrent = 125.25f;
    mState.data.inverter.enabled = VEHICLESTATE_INVERTER_ENABLED;
    mState.data.inverter.dcBusVoltage = 624.5f;

    VehicleState_Data_T destData;
    bool status = VehicleState_CopyState(&mState, &destData);
    TEST_ASSERT_TRUE(status);
    TEST_ASSERT_EQUAL_MEMORY(&mState.data, &destData, sizeof(VehicleState_Data_T));
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, CopyStateFail)
{
    mockSemaphoreSetLocked(true);
    VehicleState_Data_T destData;
    bool status = VehicleState_CopyState(&mState, &destData);
    TEST_ASSERT_FALSE(status);

    mockSemaphoreSetLocked(false);
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, AccessAcquire)
{
    mockSemaphoreSetLocked(false);
    TEST_ASSERT_TRUE(VehicleState_AccessAcquire(&mState));
    TEST_ASSERT_TRUE(mockSempahoreGetLocked());

    TEST_ASSERT_FALSE(VehicleState_AccessAcquire(&mState));
    TEST_ASSERT_TRUE(mockSempahoreGetLocked());

    mockSemaphoreSetLocked(false);
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, AccessRelease)
{
    mockSemaphoreSetLocked(false);
    TEST_ASSERT_FALSE(VehicleState_AccessRelease(&mState));
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());

    mockSemaphoreSetLocked(true);
    TEST_ASSERT_TRUE(VehicleState_AccessRelease(&mState));
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());
}

TEST_GROUP_RUNNER(VEHICLEINTERFACE_VEHICLESTATE)
{
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, InitOk);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, CopyState);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, CopyStateFail);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, AccessAcquire);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, AccessRelease);
}

#define INVOKE_TEST VEHICLEINTERFACE_VEHICLESTATE
#include "test_main.h"
