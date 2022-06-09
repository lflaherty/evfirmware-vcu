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
static VehicleState_T mVehicleState;
static Config_T mConfig;

static const uint32_t tickRateMs = 10; // 100Hz

// Set up config constants
static const uint16_t accelALower = 1000U;
static const uint16_t accelAUpper = 2000U;
static const uint16_t accelBLower = 3000U;
static const uint16_t accelBUpper = 4000U;
static const float accelPedalConsistencyPct = 0.1f; // 10%
static const uint16_t pedalInvalidTimeout = 100U; // 100ms

static void stepAndAssert(FaultStatus_T status, uint16_t steps)
{
    for (uint16_t i = 0; i < steps; ++i) {
        FaultStatus_T faultStatus = FaultManager_Step(&mFaultMgr);
        TEST_ASSERT_EQUAL(status, faultStatus);
    }
}

static void resetFaults(void)
{
    mFaultMgr.internal.faults = FAULTMGR_NO_FAULT;
    mFaultMgr.internal.accelPedalRangeTimer = 0U;
    mFaultMgr.internal.pedalConsistencyTimer = 0U;
}

static void setValidData(void)
{
    memset(&mVehicleState.data, 0U, sizeof(VehicleState_Data_T));
    mVehicleState.data.inputs.accelRawA = accelALower;
    mVehicleState.data.inputs.accelRawB = accelBLower;
}

TEST_GROUP(VEHICLELOGIC_FAULTMANAGER);

TEST_SETUP(VEHICLELOGIC_FAULTMANAGER)
{
    testLog.enableLogToDebug = true;
    testLog.enableLogToLogFile = false;
    testLog.enableLogToSerial = false;
    mockLogClear();

    // Set up config
    memset(&mConfig, 0U, sizeof(Config_T));
    mConfig.inputs.accelPedal.calibrationA.rawLower = accelALower;
    mConfig.inputs.accelPedal.calibrationA.rawUpper = accelAUpper;
    mConfig.inputs.accelPedal.calibrationB.rawLower = accelBLower;
    mConfig.inputs.accelPedal.calibrationB.rawUpper = accelBUpper;
    mConfig.inputs.accelPedal.consistencyLimit = accelPedalConsistencyPct;
    mConfig.inputs.accelPedal.invalidDataTimeout = pedalInvalidTimeout;

    // Set up fault manager
    memset(&mFaultMgr, 0U, sizeof(FaultManager_T));
    mFaultMgr.tickRateMs = tickRateMs;
    mFaultMgr.vehicleConfig = &mConfig;
    mFaultMgr.vehicleData = &mVehicleState;

    VehicleState_Init(&testLog, &mVehicleState);
    mockLogClear(); // clear again to get rid of VehicleState logs

    FaultManager_Init(&testLog, &mFaultMgr);

    const char* expectedLogging =
        "FaultManager_Init begin\n"
        "FaultManager_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for coming tests
    mockLogClear();

    // start the test case with non-fault data
    setValidData();
}

TEST_TEAR_DOWN(VEHICLELOGIC_FAULTMANAGER)
{
    mockLogClear();
}

TEST(VEHICLELOGIC_FAULTMANAGER, InitOk)
{
    // Done by TEST_SETUP
}

TEST(VEHICLELOGIC_FAULTMANAGER, FaultAccelPedalRange)
{
    uint16_t timeoutCount = pedalInvalidTimeout / tickRateMs;

    mVehicleState.data.inputs.accelRawA = 1500U;
    mVehicleState.data.inputs.accelRawB = 3500U;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    mVehicleState.data.inputs.accelRawA = 2500U;
    mVehicleState.data.inputs.accelRawB = 3500U;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    resetFaults();
    mVehicleState.data.inputs.accelRawA = 1500U;
    mVehicleState.data.inputs.accelRawB = 4500U;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    resetFaults();
    mVehicleState.data.inputs.accelRawA = 2500U;
    mVehicleState.data.inputs.accelRawB = 4500U;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    mVehicleState.data.inputs.accelRawA = 1500U;
    mVehicleState.data.inputs.accelRawB = 3500U;
    stepAndAssert(FAULT_FAULT, timeoutCount); // faults should latch
}

TEST(VEHICLELOGIC_FAULTMANAGER, FaultAccelPedalConsistency)
{
    uint16_t timeoutCount = pedalInvalidTimeout / tickRateMs;

    mVehicleState.data.inputs.accelA = 0.0f;
    mVehicleState.data.inputs.accelB = 0.0f;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    mVehicleState.data.inputs.accelA = 0.7f;
    mVehicleState.data.inputs.accelB = 0.7f;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    // 5% difference
    mVehicleState.data.inputs.accelA = 0.0f;
    mVehicleState.data.inputs.accelB = 0.05f;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    // 10% difference
    mVehicleState.data.inputs.accelA = 0.0f;
    mVehicleState.data.inputs.accelB = 0.1f;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    // 15% difference
    mVehicleState.data.inputs.accelA = 0.0f;
    mVehicleState.data.inputs.accelB = 0.15f;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    // 0% difference (but it latches)
    mVehicleState.data.inputs.accelA = 0.0f;
    mVehicleState.data.inputs.accelB = 0.0f;
    stepAndAssert(FAULT_FAULT, timeoutCount);
}

TEST_GROUP_RUNNER(VEHICLELOGIC_FAULTMANAGER)
{
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, InitOk);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultAccelPedalRange);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultAccelPedalConsistency);
}
