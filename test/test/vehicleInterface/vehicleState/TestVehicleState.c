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
    mockClearQueueData();
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
    mockClearQueueData();
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, InitOk)
{
    // Done by TEST_SETUP
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, InitTaskRegisterError)
{
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_ERROR_FULL);

    VehicleState_Status_T status = VehicleState_Init(&testLog, &mState);
    TEST_ASSERT(VEHICLESTATE_STATUS_ERROR_INIT == status);

    const char* expectedLogging =
        "VehicleState_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, PushField)
{
    // External call:
    uint32_t value = 0x0AF0U;
    VehicleState_Status_T status = VehicleState_PushField(&mState, &mState.data.inverter.runFaults, &value, sizeof(uint32_t));
    TEST_ASSERT_EQUAL(status, VEHICLESTATE_STATUS_OK);

    // Internal sequence of events:
    mockSetTaskNotifyValue(1); // to wake up
    StateProcessing(&mState); // RTOS will eventually call this

    TEST_ASSERT_EQUAL(value, mState.data.inverter.runFaults);
    TEST_ASSERT_EQUAL(0x0U, mState.data.inverter.postFaults);
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, PushFieldTooLarge)
{
    // External call:
    uint32_t value = 0x0AA0U;
    VehicleState_Status_T status = VehicleState_PushField(&mState, &mState.data.inverter.runFaults, &value, VEHICLESTATE_QUEUE_MAX_VALUE_SIZE+1);
    TEST_ASSERT_EQUAL(status, VEHICLESTATE_STATUS_ERROR_SIZE);

    // Internal sequence of events:
    mockSetTaskNotifyValue(1); // to wake up
    StateProcessing(&mState); // RTOS will eventually call this

    // This time, there should be no update
    TEST_ASSERT_EQUAL(0x0U, mState.data.inverter.runFaults);
    TEST_ASSERT_EQUAL(0x0U, mState.data.inverter.postFaults);
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, PushFieldQueueFull)
{
    uint32_t value1 = 0x0AA0U;
    uint32_t value2 = 0x0AB0U;
    VehicleState_Status_T status;

    // Fill up queue
    for (uint32_t i = 0; i < VEHICLESTATE_QUEUE_LENGTH; ++i) {
        status = VehicleState_PushField(&mState, &mState.data.inverter.runFaults, &value1, sizeof(uint32_t));
        TEST_ASSERT_EQUAL(status, VEHICLESTATE_STATUS_OK);
    }

    status = VehicleState_PushField(&mState, &mState.data.inverter.runFaults, &value2, sizeof(uint32_t));
    TEST_ASSERT_EQUAL(status, VEHICLESTATE_STATUS_ERROR_QUEUE);

    mockSetTaskNotifyValue(1); // to wake up
    StateProcessing(&mState); // RTOS will eventually call this

    TEST_ASSERT_EQUAL(value1, mState.data.inverter.runFaults);
    TEST_ASSERT_EQUAL(0x0U, mState.data.inverter.postFaults);
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, PushFieldf)
{
    // External call:
    float value = 420.0f;
    VehicleState_Status_T status = VehicleState_PushFieldf(&mState, &mState.data.motor.calculatedTorque, value);
    TEST_ASSERT_EQUAL(status, VEHICLESTATE_STATUS_OK);

    // Internal sequence of events:
    mockSetTaskNotifyValue(1); // to wake up
    StateProcessing(&mState); // RTOS will eventually call this

    TEST_ASSERT_EQUAL_FLOAT(value, mState.data.motor.calculatedTorque);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, mState.data.motor.phaseCCurrent);
}

TEST(VEHICLEINTERFACE_VEHICLESTATE, PushFieldfQueueFull)
{
    float value1 = 420.0f;
    float value2 = 422.0f;
    VehicleState_Status_T status;

    // Fill up queue
    for (uint32_t i = 0; i < VEHICLESTATE_QUEUE_LENGTH; ++i) {
        status = VehicleState_PushFieldf(&mState, &mState.data.motor.calculatedTorque, value1);
        TEST_ASSERT_EQUAL(status, VEHICLESTATE_STATUS_OK);
    }

    status = VehicleState_PushFieldf(&mState, &mState.data.motor.calculatedTorque, value2);
    TEST_ASSERT_EQUAL(status, VEHICLESTATE_STATUS_ERROR_QUEUE);

    mockSetTaskNotifyValue(1); // to wake up
    StateProcessing(&mState); // RTOS will eventually call this

    TEST_ASSERT_EQUAL_FLOAT(value1, mState.data.motor.calculatedTorque);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, mState.data.motor.phaseCCurrent);
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
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, InitTaskRegisterError);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, PushField);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, PushFieldTooLarge);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, PushFieldQueueFull);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, PushFieldf);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, PushFieldfQueueFull);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, CopyState);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, CopyStateFail);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, AccessAcquire);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLESTATE, AccessRelease);
}
