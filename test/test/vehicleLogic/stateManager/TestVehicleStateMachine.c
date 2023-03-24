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
#include "vehicleLogic/stateManager/MockFaultManager.h"
#include "vehicleLogic/throttleController/MockThrottleController.h"

// source code under test
#include "vehicleLogic/stateManager/stateMachine.c"


static Config_T mConfig;
static const uint32_t ticksPerMs = 10; // 100Hz
static const uint32_t hvActiveWait = 3*1000U; // 3 seconds (in ms)
static const uint32_t hvChargeTimeoutMs = 5*1000; // 5 seconds (in ms)

static Logging_T testLog;
static VehicleState_T mVehicleState;
static ThrottleController_T mThrottleController;
static VSM_T mVsm;

static void setVsmState(VSM_State_T state, uint32_t nTicks)
{
    mVsm.vsmState = state;
    mVsm.nextState = state;
    mVsm.ticksInState = nTicks;
}

/**
 * @brief Steps the state a number of times and checks that the state
 * holds at the given value
 * 
 * @param state 
 */
static void stepAndAssertStable(VSM_State_T state)
{
    VSM_Step(&mVsm);
    for (int i = 0; i < 12; ++i) {
        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(state, mVsm.vsmState);
    }
}

/**
 * @brief Steps the state a number of times and checks that the state
 * holds at the given value
 * 
 * @param state 
 */
static void stepAndAssertStable2(
        VSM_State_T state,
        bool torqueEnabled,
        VehicleState_InverterDirection_T direction)
{
    VSM_Step(&mVsm);
    for (int i = 0; i < 12; ++i) {
        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(state, mVsm.vsmState);
        TEST_ASSERT_EQUAL(direction, mockGet_ThrottleController_MotorDirection());
        TEST_ASSERT_EQUAL(torqueEnabled, mockGet_ThrottleController_TorqueEnable());
    }
}

TEST_GROUP(VEHICLELOGIC_STATEMACHINE);

TEST_SETUP(VEHICLELOGIC_STATEMACHINE)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();

    // set LV error since system should start this way
    mockSet_FaultManager_Step_Status(FAULT_LV_ERROR);

    mockSet_ThrottleController_TorqueEnable(false);
    mockSet_ThrottleController_MotorDirection(VEHICLESTATE_INVERTER_FORWARD);

    memset(&mConfig, 0U, sizeof(Config_T));
    mConfig.vcu.hvActiveStateWait = hvActiveWait;
    mConfig.vcu.hvChargeTimeout = hvChargeTimeoutMs;

    memset(&mVsm, 0xFF, sizeof(VSM_T));
    memset(&mVehicleState, 0x00, sizeof(VehicleState_T));
    mVsm.tickRateMs = ticksPerMs;
    mVsm.inputState = &mVehicleState;
    mVsm.vehicleConfig = &mConfig;
    mVsm.throttleController = &mThrottleController;
    mVehicleState.data.inverter.state = VEHICLESTATE_INVERTERSTATE_START;

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
    stepAndAssertStable(VSM_STATE_LV_STARTUP);

    // Transition to no fault (LV has started correctly)
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);

    // State should stay in LV ready
    stepAndAssertStable(VSM_STATE_LV_READY);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateLvStartupFault)
{
    // Start in LV Startup state
    setVsmState(VSM_STATE_LV_STARTUP, 0);

    // Stay in LV startup state while LV errors are present
    stepAndAssertStable(VSM_STATE_LV_STARTUP);

    // Transition to a HV fault
    mockSet_FaultManager_Step_Status(FAULT_FAULT);

    // State should stay in fault
    stepAndAssertStable(VSM_STATE_FAULT);

    // Transitioning out of a HV fault shouldn't change the state
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);
    stepAndAssertStable(VSM_STATE_FAULT);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateLvReadyOk)
{
    // Start in LV Ready state & no faults
    setVsmState(VSM_STATE_LV_READY, 0);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);
    mVehicleState.data.dash.buttonPressed = false;

    // Stay in LV ready state while input button hasn't been pressed
    stepAndAssertStable(VSM_STATE_LV_READY);

    // Request HV charge
    mVehicleState.data.dash.buttonPressed = true;

    // Stay in LV ready state while input button hasn't been pressed
    stepAndAssertStable(VSM_STATE_HV_ACTIVE);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateLvReadyFault)
{
    // Start in LV Ready state & no faults
    setVsmState(VSM_STATE_LV_READY, 0);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);

    // Stay in LV ready state while input button hasn't been pressed
    stepAndAssertStable(VSM_STATE_LV_READY);

    // Transition to a HV fault
    mockSet_FaultManager_Step_Status(FAULT_FAULT);

    // Stay in LV ready state while input button hasn't been pressed
    stepAndAssertStable(VSM_STATE_FAULT);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateHvActiveOk)
{
    // Start in HV charging state & no faults
    setVsmState(VSM_STATE_HV_ACTIVE, 0);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);

    // Stay in HV active state while waiting
    uint32_t hvChargeWaitTicks = 3000U / ticksPerMs;
    for (uint32_t i = 0; i < hvChargeWaitTicks; ++i) {
        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(VSM_STATE_HV_ACTIVE, mVsm.vsmState);
    }

    // State transitions to active - neutral
    stepAndAssertStable(VSM_STATE_HV_CHARGING);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateHvActiveFault)
{
    // Start in HV charging state & no faults
    setVsmState(VSM_STATE_HV_ACTIVE, 0);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);

    // Stay in HV active state while waiting
    stepAndAssertStable(VSM_STATE_HV_ACTIVE);

    // Transition to a HV fault
    mockSet_FaultManager_Step_Status(FAULT_FAULT);

    // State should stay in fault
    stepAndAssertStable(VSM_STATE_FAULT);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateHvChargingOk)
{
    // Start in HV charging state & no faults
    setVsmState(VSM_STATE_HV_CHARGING, 0);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);
    mVehicleState.data.inverter.state = VEHICLESTATE_INVERTERSTATE_START;

    // Stay in HV charging state for a bit (while charging)
    stepAndAssertStable(VSM_STATE_HV_CHARGING);

    mVehicleState.data.inverter.state = VEHICLESTATE_INVERTERSTATE_READY;

    // State transitions to active - neutral
    stepAndAssertStable(VSM_STATE_ACTIVE_NEUTRAL);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateHvChargingFault)
{
    // Start in HV charging state & no faults
    setVsmState(VSM_STATE_HV_CHARGING, 0);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);
    mVehicleState.data.inverter.state = VEHICLESTATE_INVERTERSTATE_PRECHARGEACTIVE;

    // Stay in HV charging state for a bit (while charging)
    stepAndAssertStable(VSM_STATE_HV_CHARGING);

    // Transition to a HV fault
    mockSet_FaultManager_Step_Status(FAULT_FAULT);

    // State should stay in fault
    stepAndAssertStable(VSM_STATE_FAULT);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateHvChargingTimeout)
{
    // Start in HV charging state & no faults
    setVsmState(VSM_STATE_HV_CHARGING, 0);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);
    mVehicleState.data.inverter.state = VEHICLESTATE_INVERTERSTATE_PRECHARGEACTIVE;

    // Stay in HV charging state while not timed out
    // (+ 1 to exceed the timeout)
    uint32_t timeoutTicks = hvChargeTimeoutMs / ticksPerMs + 1;
    for (uint32_t i = 0; i < timeoutTicks; ++i) {
        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(VSM_STATE_HV_CHARGING, mVsm.vsmState);
    }

    // Should timeout and go to a fault
    stepAndAssertStable(VSM_STATE_FAULT);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateActiveNeutral)
{
    // Start in HV charging state & no faults
    setVsmState(VSM_STATE_ACTIVE_NEUTRAL, 0);
    mockSet_ThrottleController_TorqueEnable(false);
    mockSet_ThrottleController_MotorDirection(VEHICLESTATE_INVERTER_FORWARD);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);

    // Stay in HV charging state for a bit (while charging)
    stepAndAssertStable2(VSM_STATE_ACTIVE_NEUTRAL, false, VEHICLESTATE_INVERTER_FORWARD);

    mVehicleState.data.dash.buttonPressed = true;

    // State transitions to active - neutral
    stepAndAssertStable2(VSM_STATE_ACTIVE_FORWARD, true, VEHICLESTATE_INVERTER_FORWARD);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateActiveNeutralFault)
{
    // Start in HV charging state & no faults
    setVsmState(VSM_STATE_ACTIVE_NEUTRAL, 0);
    mockSet_ThrottleController_TorqueEnable(false);
    mockSet_ThrottleController_MotorDirection(VEHICLESTATE_INVERTER_FORWARD);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);

    // Stay in HV charging state for a bit (while charging)
    stepAndAssertStable2(VSM_STATE_ACTIVE_NEUTRAL, false, VEHICLESTATE_INVERTER_FORWARD);

    // Transition to a HV fault
    mockSet_FaultManager_Step_Status(FAULT_FAULT);

    // State should stay in fault
    stepAndAssertStable2(VSM_STATE_FAULT, false, VEHICLESTATE_INVERTER_FORWARD);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateActiveForward)
{
    // Start in HV charging state & no faults
    setVsmState(VSM_STATE_ACTIVE_FORWARD, 0);
    mockSet_ThrottleController_TorqueEnable(true);
    mockSet_ThrottleController_MotorDirection(VEHICLESTATE_INVERTER_FORWARD);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);

    // Stay in HV charging state for a bit (while charging)
    stepAndAssertStable2(VSM_STATE_ACTIVE_FORWARD, true, VEHICLESTATE_INVERTER_FORWARD);

    mVehicleState.data.dash.buttonPressed = true;

    // State transitions to active - neutral
    stepAndAssertStable2(VSM_STATE_ACTIVE_NEUTRAL, false, VEHICLESTATE_INVERTER_FORWARD);
}

TEST(VEHICLELOGIC_STATEMACHINE, StateActiveForwardFault)
{
    // Start in HV charging state & no faults
    setVsmState(VSM_STATE_ACTIVE_FORWARD, 0);
    mockSet_ThrottleController_TorqueEnable(true);
    mockSet_ThrottleController_MotorDirection(VEHICLESTATE_INVERTER_FORWARD);
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);

    // Stay in HV charging state for a bit (while charging)
    stepAndAssertStable2(VSM_STATE_ACTIVE_FORWARD, true, VEHICLESTATE_INVERTER_FORWARD);

    // Transition to a HV fault
    mockSet_FaultManager_Step_Status(FAULT_FAULT);

    // State should stay in fault
    stepAndAssertStable2(VSM_STATE_FAULT, false, VEHICLESTATE_INVERTER_FORWARD);
}

TEST(VEHICLELOGIC_STATEMACHINE, StartupProcedure)
{
    // Test the full sequence from init to active - forward

    // 1. Init
    TEST_ASSERT_EQUAL(VSM_STATE_INIT, mVsm.vsmState);

    // 2. LV startup
    VSM_Step(&mVsm);
    stepAndAssertStable2(VSM_STATE_LV_STARTUP, false, VEHICLESTATE_INVERTER_FORWARD);

    // 3. LV ready
    mockSet_FaultManager_Step_Status(FAULT_NO_FAULT);
    VSM_Step(&mVsm);
    stepAndAssertStable2(VSM_STATE_LV_READY, false, VEHICLESTATE_INVERTER_FORWARD);

    // 4. HV active
    mVehicleState.data.dash.buttonPressed = true;

    uint32_t hvChargeWaitTicks = 3000U / ticksPerMs + 1; // (+1 to transition into state)
    uint32_t buttonSwitchOffTime = 1000U / ticksPerMs;
    for (uint32_t i = 0; i < hvChargeWaitTicks; ++i) {
        if (i > buttonSwitchOffTime) {
            // release the button
            mVehicleState.data.dash.buttonPressed = false;
        }

        VSM_Step(&mVsm);
        TEST_ASSERT_EQUAL(VSM_STATE_HV_ACTIVE, mVsm.vsmState);
    }

    // 5. HV charging
    stepAndAssertStable2(VSM_STATE_HV_CHARGING, false, VEHICLESTATE_INVERTER_FORWARD);

    // 6. Active - neutral
    mVehicleState.data.inverter.state = VEHICLESTATE_INVERTERSTATE_READY;
    stepAndAssertStable2(VSM_STATE_ACTIVE_NEUTRAL, false, VEHICLESTATE_INVERTER_FORWARD);

    // 7. Active - forward
    mVehicleState.data.dash.buttonPressed = true;
    stepAndAssertStable2(VSM_STATE_ACTIVE_FORWARD, true, VEHICLESTATE_INVERTER_FORWARD);
    mVehicleState.data.dash.buttonPressed = false;
    stepAndAssertStable2(VSM_STATE_ACTIVE_FORWARD, true, VEHICLESTATE_INVERTER_FORWARD);

    // 8. Active - neutral
    mVehicleState.data.dash.buttonPressed = true;
    stepAndAssertStable2(VSM_STATE_ACTIVE_NEUTRAL, false, VEHICLESTATE_INVERTER_FORWARD);
    mVehicleState.data.dash.buttonPressed = false;
    stepAndAssertStable2(VSM_STATE_ACTIVE_NEUTRAL, false, VEHICLESTATE_INVERTER_FORWARD);
}

TEST_GROUP_RUNNER(VEHICLELOGIC_STATEMACHINE)
{
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, InitOk);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, InvalidState);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateInit);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateLvStartupOk);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateLvStartupFault);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateLvReadyOk);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateLvReadyFault);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateHvActiveOk);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateHvActiveFault);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateHvChargingOk);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateHvChargingFault);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateHvChargingTimeout);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateActiveNeutral);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateActiveNeutralFault);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateActiveForward);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StateActiveForwardFault);
    RUN_TEST_CASE(VEHICLELOGIC_STATEMACHINE, StartupProcedure);
}

#define INVOKE_TEST VEHICLELOGIC_STATEMACHINE
#include "test_main.h"
