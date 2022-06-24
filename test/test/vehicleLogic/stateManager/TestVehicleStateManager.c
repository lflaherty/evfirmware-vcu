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
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"
#include "time/tasktimer/MockTasktimer.h"
#include "vehicleInterface/vehicleControl/MockVehicleControl.h"
#include "vehicleLogic/stateManager/MockStateMachine.h"
#include "vehicleLogic/throttleController/MockThrottleController.h"

// source code under test
#include "vehicleLogic/stateManager/vehicleStateManager.c"

static Logging_T testLog;
static VehicleStateManager_T mStateMgr;
static VehicleState_T mInputState;
static VehicleControl_T mControl;
static ThrottleController_T mThrottleController;
static Config_T mConfig;

TEST_GROUP(VEHICLELOGIC_VEHICLESTATEMANAGER);

TEST_SETUP(VEHICLELOGIC_VEHICLESTATEMANAGER)
{
    testLog.enableLogToDebug = true;
    testLog.enableLogToLogFile = false;
    testLog.enableLogToSerial = false;
    mockLogClear();
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);

    memset(&mStateMgr, 0U, sizeof(VehicleStateManager_T));
    memset(&mInputState, 0U, sizeof(VehicleState_T));
    memset(&mControl, 0U, sizeof(VehicleControl_T));
    memset(&mConfig, 0U, sizeof(Config_T));

    mStateMgr.inputState = &mInputState;
    mStateMgr.control = &mControl;
    mStateMgr.throttleController = &mThrottleController;
    mStateMgr.vehicleConfig = &mConfig;
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

TEST(VEHICLELOGIC_VEHICLESTATEMANAGER, InitTaskRegisterError)
{
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_ERROR_FULL);

    VehicleStateManager_Status_T status = VehicleStateManager_Init(&testLog, &mStateMgr);
    TEST_ASSERT(STATEMANAGER_STATUS_ERROR_INIT == status);

    const char* expectedLogging =
        "VehicleStateManager_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(VEHICLELOGIC_VEHICLESTATEMANAGER, VsmCalled)
{
    uint32_t expectedSteps = 0U;
    TEST_ASSERT_EQUAL(expectedSteps, mockGet_VSM_Step_Count());

    StateManagerProcessing(&mStateMgr);
    TEST_ASSERT_EQUAL(expectedSteps, mockGet_VSM_Step_Count());

    for (uint32_t i = 0; i < 100U; ++i) {
        expectedSteps++;
        mockSetTaskNotifyValue(1); // to wake up
        for (uint32_t j = 0; j < 100U; ++j) {
            StateManagerProcessing(&mStateMgr); // RTOS will eventually call this if it is notified
            TEST_ASSERT_EQUAL(expectedSteps, mockGet_VSM_Step_Count());
        }
    }
}

TEST_GROUP_RUNNER(VEHICLELOGIC_VEHICLESTATEMANAGER)
{
    RUN_TEST_CASE(VEHICLELOGIC_VEHICLESTATEMANAGER, InitOk);
    RUN_TEST_CASE(VEHICLELOGIC_VEHICLESTATEMANAGER, InitTaskRegisterError);
    RUN_TEST_CASE(VEHICLELOGIC_VEHICLESTATEMANAGER, VsmCalled);
}
