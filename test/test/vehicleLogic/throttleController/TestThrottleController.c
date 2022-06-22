/**
 * TestThrottleController.c
 * 
 *  Created on: Jun 19 2022
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

// source code under test
#include "vehicleLogic/throttleController/throttleController.c"

static Logging_T testLog;
static ThrottleController_T mThrottleController;
static VehicleState_T mInputState;
static VehicleControl_T mControl;
static Config_T mConfig;

TEST_GROUP(VEHICLELOGIC_THROTTLECONTROLLER);

TEST_SETUP(VEHICLELOGIC_THROTTLECONTROLLER)
{
    testLog.enableLogToDebug = true;
    testLog.enableLogToLogFile = false;
    testLog.enableLogToSerial = false;
    mockLogClear();
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);
    mockReset_VehicleControl_RequestMotorTorque();
    mockSet_VehicleControl_RequestMotorTorque_LastRequest(0.0f);
    mockSet_VehicleControl_RequestMotorTorque_LastRequestDir(VEHICLESTATE_INVERTER_REVERSE);

    memset(&mThrottleController, 0U, sizeof(ThrottleController_T));
    memset(&mInputState, 0U, sizeof(VehicleState_T));
    memset(&mControl, 0U, sizeof(VehicleControl_T));
    memset(&mConfig, 0U, sizeof(Config_T));

    mThrottleController.inputState = &mInputState;
    mThrottleController.control = &mControl;
    mThrottleController.vehicleConfig = &mConfig;
    ThrottleController_Status_T status = ThrottleController_Init(&testLog, &mThrottleController);
    TEST_ASSERT(THROTTLECONTROLLER_STATUS_OK == status);

    const char* expectedLogging =
        "ThrottleController_Init begin\n"
        "ThrottleController_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for coming tests
    mockLogClear();
}

TEST_TEAR_DOWN(VEHICLELOGIC_THROTTLECONTROLLER)
{
    mockLogClear();
}

TEST(VEHICLELOGIC_THROTTLECONTROLLER, InitOk)
{
    // Done by TEST_SETUP
}

TEST(VEHICLELOGIC_THROTTLECONTROLLER, InitTaskRegisterError)
{
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_ERROR_FULL);

    ThrottleController_Status_T status = ThrottleController_Init(&testLog, &mThrottleController);
    TEST_ASSERT(THROTTLECONTROLLER_STATUS_ERROR_INIT == status);

    const char* expectedLogging =
        "ThrottleController_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(VEHICLELOGIC_THROTTLECONTROLLER, TorqueOutputDisabled)
{
    uint32_t expectedTorqueRequests = 0U;

    // Set throttle pedal
    mInputState.data.inputs.accel = 0.0f;

    mockSetTaskNotifyValue(1); // to wake up
    ThrottleController(&mThrottleController); // RTOS will eventually call this
    expectedTorqueRequests++;

    TEST_ASSERT_EQUAL(expectedTorqueRequests, mockGet_VehicleControl_RequestMotorTorque_NumRequests());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, mockGet_VehicleControl_RequestMotorTorque_LastRequest());

    // Make sure it really is disabled
    mInputState.data.inputs.accel = 1.0f;

    mockSetTaskNotifyValue(1); // to wake up
    ThrottleController(&mThrottleController); // RTOS will eventually call this
    expectedTorqueRequests++;

    TEST_ASSERT_EQUAL(expectedTorqueRequests, mockGet_VehicleControl_RequestMotorTorque_NumRequests());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, mockGet_VehicleControl_RequestMotorTorque_LastRequest());
}

TEST(VEHICLELOGIC_THROTTLECONTROLLER, BadDirection)
{
    uint32_t expectedTorqueRequests = 0U;
    
    ThrottleController_SetTorqueEnabled(&mThrottleController, true);
    ThrottleController_SetMotorDirection(&mThrottleController, 4U);

    // Set throttle pedal
    mInputState.data.inputs.accel = 0.0f;

    mockSetTaskNotifyValue(1); // to wake up
    ThrottleController(&mThrottleController); // RTOS will eventually call this
    expectedTorqueRequests++;

    TEST_ASSERT_EQUAL(expectedTorqueRequests, mockGet_VehicleControl_RequestMotorTorque_NumRequests());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, mockGet_VehicleControl_RequestMotorTorque_LastRequest());

    // Make sure no torque is requested
    mInputState.data.inputs.accel = 1.0f;

    mockSetTaskNotifyValue(1); // to wake up
    ThrottleController(&mThrottleController); // RTOS will eventually call this
    expectedTorqueRequests++;

    TEST_ASSERT_EQUAL(expectedTorqueRequests, mockGet_VehicleControl_RequestMotorTorque_NumRequests());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, mockGet_VehicleControl_RequestMotorTorque_LastRequest());
}

TEST(VEHICLELOGIC_THROTTLECONTROLLER, TorqueOutputForward)
{
    uint32_t expectedTorqueRequests = 0U;
    
    ThrottleController_SetTorqueEnabled(&mThrottleController, true);

    // Replicate torque map test here
    float inputs[] =   {0.0f, 0.05f, 0.1f,  0.2f,  0.4f,   0.5f,   0.6f,   0.7f,   0.8f,   0.9f,   1.0f};
    float expected[] = {0.0f, 0.0f,  0.0f, 25.0f, 75.0f, 100.0f, 150.0f, 200.0f, 300.0f, 400.0f, 500.0f};
    size_t testLen = sizeof(inputs) / sizeof(float);

    for (size_t i = 0; i < testLen; ++i) {
        mInputState.data.inputs.accel = inputs[i];

        mockSetTaskNotifyValue(1); // to wake up
        ThrottleController(&mThrottleController); // RTOS will eventually call this
        expectedTorqueRequests++;

        TEST_ASSERT_EQUAL(expectedTorqueRequests, mockGet_VehicleControl_RequestMotorTorque_NumRequests());
        TEST_ASSERT_EQUAL_FLOAT(expected[i], mockGet_VehicleControl_RequestMotorTorque_LastRequest());
    }

    // Disable and check that no torque is output
    ThrottleController_SetTorqueEnabled(&mThrottleController, false);
    mInputState.data.inputs.accel = 1.0f;

    mockSetTaskNotifyValue(1); // to wake up
    ThrottleController(&mThrottleController); // RTOS will eventually call this
    expectedTorqueRequests++;

    TEST_ASSERT_EQUAL(expectedTorqueRequests, mockGet_VehicleControl_RequestMotorTorque_NumRequests());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, mockGet_VehicleControl_RequestMotorTorque_LastRequest());
}

TEST(VEHICLELOGIC_THROTTLECONTROLLER, TorqueOutputReverse)
{
    uint32_t expectedTorqueRequests = 0U;
    
    ThrottleController_SetTorqueEnabled(&mThrottleController, true);
    ThrottleController_SetMotorDirection(&mThrottleController, VEHICLESTATE_INVERTER_REVERSE);

    // Replicate torque map test here
    float inputs[] =   {0.0f, 0.05f, 0.1f, 0.2f,  0.4f,  0.5f,  0.6f,  0.7f,  0.8f,  0.9f,  1.0f};
    float expected[] = {0.0f, 0.0f,  0.0f, 5.0f, 15.0f, 20.0f, 47.5f, 75.0f, 75.0f, 75.0f, 75.0f};
    size_t testLen = sizeof(inputs) / sizeof(float);

    for (size_t i = 0; i < testLen; ++i) {
        mInputState.data.inputs.accel = inputs[i];

        mockSetTaskNotifyValue(1); // to wake up
        ThrottleController(&mThrottleController); // RTOS will eventually call this
        expectedTorqueRequests++;

        TEST_ASSERT_EQUAL(expectedTorqueRequests, mockGet_VehicleControl_RequestMotorTorque_NumRequests());
        TEST_ASSERT_EQUAL_FLOAT(expected[i], mockGet_VehicleControl_RequestMotorTorque_LastRequest());
    }

    // Disable and check that no torque is output
    ThrottleController_SetTorqueEnabled(&mThrottleController, false);
    mInputState.data.inputs.accel = 1.0f;

    mockSetTaskNotifyValue(1); // to wake up
    ThrottleController(&mThrottleController); // RTOS will eventually call this
    expectedTorqueRequests++;

    TEST_ASSERT_EQUAL(expectedTorqueRequests, mockGet_VehicleControl_RequestMotorTorque_NumRequests());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, mockGet_VehicleControl_RequestMotorTorque_LastRequest());
}

TEST(VEHICLELOGIC_THROTTLECONTROLLER, SetTorqueEnabled)
{
    ThrottleController_Status_T status;

    mockSemaphoreSetLocked(false);
    status = ThrottleController_SetTorqueEnabled(&mThrottleController, true);
    TEST_ASSERT_EQUAL(THROTTLECONTROLLER_STATUS_OK, status);
    TEST_ASSERT_TRUE(mThrottleController.enabled);
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());

    mockSemaphoreSetLocked(true);
    status = ThrottleController_SetTorqueEnabled(&mThrottleController, false);
    TEST_ASSERT_EQUAL(THROTTLECONTROLLER_STATUS_ERROR_MUTEX, status);
    TEST_ASSERT_TRUE(mThrottleController.enabled); // still true despite trying to set it false
    TEST_ASSERT_TRUE(mockSempahoreGetLocked());
}

TEST(VEHICLELOGIC_THROTTLECONTROLLER, SetMotorDirection)
{
    ThrottleController_Status_T status;

    mockSemaphoreSetLocked(false);
    status = ThrottleController_SetMotorDirection(&mThrottleController, VEHICLESTATE_INVERTER_FORWARD);
    TEST_ASSERT_EQUAL(THROTTLECONTROLLER_STATUS_OK, status);
    TEST_ASSERT_EQUAL(VEHICLESTATE_INVERTER_FORWARD, mThrottleController.direction);
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());

    mockSemaphoreSetLocked(true);
    status = ThrottleController_SetMotorDirection(&mThrottleController, VEHICLESTATE_INVERTER_FORWARD);
    TEST_ASSERT_EQUAL(THROTTLECONTROLLER_STATUS_ERROR_MUTEX, status);
    TEST_ASSERT_EQUAL(VEHICLESTATE_INVERTER_FORWARD, mThrottleController.direction); // still true despite trying to set it false
    TEST_ASSERT_TRUE(mockSempahoreGetLocked());
}

TEST_GROUP_RUNNER(VEHICLELOGIC_THROTTLECONTROLLER)
{
    RUN_TEST_CASE(VEHICLELOGIC_THROTTLECONTROLLER, InitOk);
    RUN_TEST_CASE(VEHICLELOGIC_THROTTLECONTROLLER, InitTaskRegisterError);
    RUN_TEST_CASE(VEHICLELOGIC_THROTTLECONTROLLER, TorqueOutputDisabled);
    RUN_TEST_CASE(VEHICLELOGIC_THROTTLECONTROLLER, BadDirection);
    RUN_TEST_CASE(VEHICLELOGIC_THROTTLECONTROLLER, TorqueOutputForward);
    RUN_TEST_CASE(VEHICLELOGIC_THROTTLECONTROLLER, TorqueOutputReverse);
    RUN_TEST_CASE(VEHICLELOGIC_THROTTLECONTROLLER, SetTorqueEnabled);
    RUN_TEST_CASE(VEHICLELOGIC_THROTTLECONTROLLER, SetMotorDirection);
}
