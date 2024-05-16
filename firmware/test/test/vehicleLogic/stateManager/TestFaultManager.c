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
#include "logging/MockLogging.h"
#include "tasktimer/MockTasktimer.h"

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
static const uint16_t brakeALower = 1000U;
static const uint16_t brakeAUpper = 2000U;
static const uint16_t brakeBLower = 3000U;
static const uint16_t brakeBUpper = 4000U;
static const uint8_t pedalAbuseCheckEnabled = 1U; // enabled
static const float pedalAbuseAccelThreshold = 0.2f; // 20%
static const float pedalAbuseBrakeThreshold = 0.2f; // 20%
static const uint16_t pedalInvalidTimeout = 100U; // 100ms
static const LowVoltage_T maxCellVoltage = 450U; // 4.5v
static const Current_T maxBatteryCurrent = 3000U; // 300A
static const Temperature_T maxCellTemperature = 900U; // 90 degrees
static const Percent_T minStateOfCharge = 500; // 5%
static const uint16_t bmsInvalidTimeout = 100u; // 100ms

static void stepAndAssert(FaultStatus_T status, uint16_t steps)
{
    for (uint16_t i = 0; i < steps; ++i) {
        FaultStatus_T faultStatus = FaultManager_Step(&mFaultMgr);
        TEST_ASSERT_EQUAL(status, faultStatus);
        TEST_ASSERT_FALSE(mockSempahoreGetLocked(mVehicleState.mutex));
    }
}

static void resetFaults(void)
{
    mFaultMgr.internal.faults = FAULTMGR_NO_FAULT;
    mFaultMgr.internal.accelPedalRangeTimer = 0U;
    mFaultMgr.internal.accelPedalConsistencyTimer = 0U;
    mFaultMgr.internal.brakePressureRangeTimer = 0U;
    mFaultMgr.internal.brakePressureConsistencyTimer = 0U;
    mFaultMgr.internal.pedalAbuseTimer = 0U;
    mFaultMgr.internal.cellTempOverTimer = 0U;
    mFaultMgr.internal.currentOverDrawTimer = 0U;
    mFaultMgr.internal.cellVoltageOverTimer = 0U;
}

static void setValidData(void)
{
    memset(&mVehicleState.data, 0U, sizeof(VehicleState_Data_T));
    mVehicleState.data.inputs.accelRawA = accelALower;
    mVehicleState.data.inputs.accelRawB = accelBLower;
    mVehicleState.data.inputs.accel = 0.0f;
    mVehicleState.data.inputs.accelA = 0.0f;
    mVehicleState.data.inputs.accelB = 0.0f;
    mVehicleState.data.inputs.brakeRawFront = brakeALower;
    mVehicleState.data.inputs.brakeRawRear = brakeBLower;
    mVehicleState.data.inputs.brakePresFront = 0.0f;
    mVehicleState.data.inputs.brakePresRear = 0.0f;
    mVehicleState.data.battery.stateOfCarge = 800U; // 80%
}

TEST_GROUP(VEHICLELOGIC_FAULTMANAGER);

TEST_SETUP(VEHICLELOGIC_FAULTMANAGER)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);

    // Set up config
    memset(&mConfig, 0U, sizeof(Config_T));
    mConfig.inputs.accelPedal.calibrationA.rawLower = accelALower;
    mConfig.inputs.accelPedal.calibrationA.rawUpper = accelAUpper;
    mConfig.inputs.accelPedal.calibrationB.rawLower = accelBLower;
    mConfig.inputs.accelPedal.calibrationB.rawUpper = accelBUpper;
    mConfig.inputs.accelPedal.consistencyLimit = accelPedalConsistencyPct;
    mConfig.inputs.brakePressureFront.rawLower = brakeALower;
    mConfig.inputs.brakePressureFront.rawUpper = brakeAUpper;
    mConfig.inputs.brakePressureRear.rawLower = brakeBLower;
    mConfig.inputs.brakePressureRear.rawUpper = brakeBUpper;
    mConfig.inputs.pedalAbuseCheckEnabled = pedalAbuseCheckEnabled;
    mConfig.inputs.pedalAbuseAccelThreshold = pedalAbuseAccelThreshold;
    mConfig.inputs.pedalAbuseBrakeThreshold = pedalAbuseBrakeThreshold;
    mConfig.inputs.invalidDataTimeout = pedalInvalidTimeout;
    mConfig.bms.maxCellTemp = maxCellTemperature;
    mConfig.bms.maxCurrent = maxBatteryCurrent;
    mConfig.bms.maxCellVoltage = maxCellVoltage;
    mConfig.bms.minStateOfCharge = minStateOfCharge;
    mConfig.bms.invalidDataTimeout = bmsInvalidTimeout;

    // Set up fault manager
    memset(&mFaultMgr, 0U, sizeof(FaultManager_T));
    mFaultMgr.tickRateMs = tickRateMs;
    mFaultMgr.vehicleConfig = &mConfig;
    mFaultMgr.vehicleData = &mVehicleState;

    TEST_ASSERT_EQUAL(VehicleState_Init(&testLog, &mVehicleState),
                      VEHICLESTATE_STATUS_OK);
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

TEST(VEHICLELOGIC_FAULTMANAGER, FaultBrakePedalRange)
{
    uint16_t timeoutCount = pedalInvalidTimeout / tickRateMs;

    mVehicleState.data.inputs.brakeRawFront = 1500U;
    mVehicleState.data.inputs.brakeRawRear = 3500U;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    mVehicleState.data.inputs.brakeRawFront = 2500U;
    mVehicleState.data.inputs.brakeRawRear = 3500U;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    resetFaults();
    mVehicleState.data.inputs.brakeRawFront = 1500U;
    mVehicleState.data.inputs.brakeRawRear = 4500U;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    resetFaults();
    mVehicleState.data.inputs.brakeRawFront = 2500U;
    mVehicleState.data.inputs.brakeRawRear = 4500U;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    mVehicleState.data.inputs.brakeRawFront = 1500U;
    mVehicleState.data.inputs.brakeRawRear = 3500U;
    stepAndAssert(FAULT_FAULT, timeoutCount); // faults should latch
}

TEST(VEHICLELOGIC_FAULTMANAGER, FaultPedalAbuse)
{
    uint16_t timeoutCount = pedalInvalidTimeout / tickRateMs;

    mVehicleState.data.inputs.accel = 0.0f;
    mVehicleState.data.inputs.brakePresFront = 0.0f;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    // one is only slightly active (below threshold)
    mVehicleState.data.inputs.accel = 0.5f;
    mVehicleState.data.inputs.brakePresFront = 0.01f;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    mVehicleState.data.inputs.accel = 0.01f;
    mVehicleState.data.inputs.brakePresFront = 0.05f;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    // they are both on
    mVehicleState.data.inputs.accel = 0.5f;
    mVehicleState.data.inputs.brakePresFront = 0.5f;
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    // 0% difference (but it latches)
    mVehicleState.data.inputs.accel = 0.0f;
    mVehicleState.data.inputs.brakePresRear = 0.0f;
    stepAndAssert(FAULT_FAULT, timeoutCount);
}

TEST(VEHICLELOGIC_FAULTMANAGER, FaultBMSCellTemperature)
{
    uint16_t timeoutCount = bmsInvalidTimeout / tickRateMs;

    // valid voltage
    mVehicleState.data.battery.maxCellTemperature = 500; // 50 deg
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    // voltage too high
    mVehicleState.data.battery.maxCellTemperature = 1000; // 100 deg
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    // fault sticks
    mVehicleState.data.battery.maxCellTemperature = 500; // 50 dec
    stepAndAssert(FAULT_FAULT, timeoutCount);
}

TEST(VEHICLELOGIC_FAULTMANAGER, FaultBMSCurrent)
{
    uint16_t timeoutCount = bmsInvalidTimeout / tickRateMs;

    // valid current
    mVehicleState.data.battery.dcCurrent = 2000; // 200 Amps
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    // current too high
    mVehicleState.data.battery.dcCurrent = 10000; // 1,000 Amps
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    // fault sticks
    mVehicleState.data.battery.dcCurrent = 0; // 0 Amps
    stepAndAssert(FAULT_FAULT, timeoutCount);
}

TEST(VEHICLELOGIC_FAULTMANAGER, FaultBMSCellVoltage)
{
    uint16_t timeoutCount = bmsInvalidTimeout / tickRateMs;

    // valid voltage
    mVehicleState.data.battery.maxCellVoltage = 360; // 3.6v
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);

    // voltage too high
    mVehicleState.data.battery.maxCellVoltage = 480; // 4.8v
    stepAndAssert(FAULT_NO_FAULT, timeoutCount);
    stepAndAssert(FAULT_FAULT, timeoutCount);

    // fault sticks
    mVehicleState.data.battery.maxCellVoltage = 360; // 3.6v
    stepAndAssert(FAULT_FAULT, timeoutCount);
}

TEST(VEHICLELOGIC_FAULTMANAGER, FaultBMSCharge)
{
    // valid charge
    mVehicleState.data.battery.stateOfCarge = 8000U; // 80.00%
    stepAndAssert(FAULT_NO_FAULT, 100U);

    // voltage too high
    mVehicleState.data.battery.stateOfCarge = 210U; // 2.10%
    stepAndAssert(FAULT_FAULT, 100U);

    // fault sticks
    mVehicleState.data.battery.stateOfCarge = 8000U; // 80%
    stepAndAssert(FAULT_FAULT, 100U);
}

TEST(VEHICLELOGIC_FAULTMANAGER, FaultBMSFaultInd)
{
    // valid charge
    mVehicleState.data.battery.bmsFaultIndicator = false;
    stepAndAssert(FAULT_NO_FAULT, 100U);

    // voltage too high
    mVehicleState.data.battery.bmsFaultIndicator = true;
    stepAndAssert(FAULT_FAULT, 100U);

    // fault sticks
    mVehicleState.data.battery.bmsFaultIndicator = false;
    stepAndAssert(FAULT_FAULT, 100U);
}

TEST_GROUP_RUNNER(VEHICLELOGIC_FAULTMANAGER)
{
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, InitOk);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultAccelPedalRange);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultAccelPedalConsistency);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultBrakePedalRange);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultPedalAbuse);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultBMSCellTemperature);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultBMSCurrent);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultBMSCellVoltage);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultBMSCharge);
    RUN_TEST_CASE(VEHICLELOGIC_FAULTMANAGER, FaultBMSFaultInd);
}

#define INVOKE_TEST VEHICLELOGIC_FAULTMANAGER
#include "test_main.h"
