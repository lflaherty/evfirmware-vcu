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
#include "Application/vehicleLogic/stateManager/MockFaultManager.h"

// source code under test
#include "vehicleLogic/stateManager/stateMachine.c"

static const uint32_t ticksPerMs = 10; // 100Hz

static Logging_T testLog;
static VehicleState_T mVehicleState;
static VSM_T mVsm;

static void setVsmState(VSM_State_T state, uint32_t nTicks)
{
    mVsm.vsmState = state;
    mVsm.nextState = state;
    mVsm.ticksInState = nTicks;
}

TEST_GROUP(VEHICLELOGIC_STATEMACHINE);

TEST_SETUP(VEHICLELOGIC_STATEMACHINE)
{
    testLog.enableLogToDebug = true;
    testLog.enableLogToLogFile = false;
    testLog.enableLogToSerial = false;
    mockLogClear();

    // set LV error since system should start this way
    mockSet_FaultManager_Step_Status(FAULT_LV_ERROR);

    memset(&mVsm, 0xFF, sizeof(VSM_T));
    memset(&mVehicleState, 0x00, sizeof(VehicleState_T));
    mVsm.tickRateMs = ticksPerMs;
    mVsm.inputState = &mVehicleState;

    VSM_Init(&testLog, &mVsm);

    const char* expectedLogging =
        "VSM_Init begin\n"
        "VSM_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
    TEST_ASSERT_EQUAL(VSM_STATE_INIT, mVsm.vsmState);
    TEST_ASSERT_EQUAL(VSM_STATE_INIT, mVsm.nextState);
    TEST_ASSERT_EQUAL(0, mVsm.ticksInState);

    // clear again for coming tests
    mockLogClear();
}

TEST_TEAR_DOWN(VEHICLELOGIC_STATEMACHINE)
{
    mockLogClear();
}

TEST(VEHICLELOGIC_STATEMACHINE, InitOk)
{
    // Done by TEST_SETUP
}

TEST(VEHICLELOGIC_STATEMACHINE, InvalidState)
{
    setVsmState(0xFF, 0);
    VSM_Step(&mVsm);
    TEST_ASSERT_EQUAL(VSM_STATE_INIT, mVsm.vsmState);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateInit)
{
    // Expect to begin in init state and transition to 
    // LV startup after one step
    TEST_ASSERT_EQUAL(VSM_STATE_INIT, mVsm.vsmState);
    VSM_Step(&mVsm);
    TEST_ASSERT_EQUAL(VSM_STATE_LV_STARTUP, mVsm.vsmState);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateLvStartupOk)
{
    // Start in LV Startup state
    setVsmState(VSM_STATE_LV_STARTUP, 0);

    // Stay in LV startup state while LV errors are present
    for (int i = 0; i < 12; ++i) {
        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(VSM_STATE_LV_STARTUP, mVsm.vsmState);
    }

    // Transition to no fault (LV has started correctly)
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);

    // State should stay in LV ready
    for (int i = 0; i < 12; ++i) {
        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(VSM_STATE_LV_READY, mVsm.vsmState);
    }
}

TEST(VEHICLELOGIC_STATEMACHINE, StateLvStartupFault)
{
    // Start in LV Startup state
    setVsmState(VSM_STATE_LV_STARTUP, 0);

    // Stay in LV startup state while LV errors are present
    for (int i = 0; i < 12; ++i) {
        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(VSM_STATE_LV_STARTUP, mVsm.vsmState);
    }

    // Transition to a HV fault
    mockSet_FaultManager_Step_Status(FAULT_FAULT);

    // State should stay in fault
    for (int i = 0; i < 12; ++i) {
        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(VSM_STATE_FAULT, mVsm.vsmState);
    }

    // Transitioning out of a HV fault shouldn't change the state
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);
    for (int i = 0; i < 12; ++i) {
        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(VSM_STATE_FAULT, mVsm.vsmState);
    }
}

TEST_GROUP_RUNNER(VEHICLELOGIC_STATEMACHINE)
{
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, InitOk);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, InvalidState);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateInit);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateLvStartupOk);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateLvStartupFault);
}
