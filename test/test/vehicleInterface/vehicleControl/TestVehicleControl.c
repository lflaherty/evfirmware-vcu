/**
 * TestVehicleControl.c
 * 
 *  Created on: May 8 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>
#include <stdio.h>

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"

#include "lib/logging/MockLogging.h"

// source code under test
#include "vehicleInterface/vehicleControl/vehicleControl.c"

static Logging_T testLog;
static VehicleControl_T mVehicleControl;

TEST_GROUP(VEHICLEINTERFACE_VEHICLECONTROL);

TEST_SETUP(VEHICLEINTERFACE_VEHICLECONTROL)
{
    testLog.enableLogToDebug = true;
    testLog.enableLogToLogFile = false;
    testLog.enableLogToSerial = false;
    mockLogClear();

    memset(&mVehicleControl, 0, sizeof(VehicleControl_T));

    VehicleControl_Status_T status = VehicleControl_Init(&testLog);
    TEST_ASSERT(VEHICLECONTORL_STATUS_OK == status);

    const char* expectedLogging =
        "VehicleControl_Init begin\n"
        "VehicleControl_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for coming tests
    mockLogClear();
}

TEST_TEAR_DOWN(VEHICLEINTERFACE_VEHICLECONTROL)
{
    mockLogClear();
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, InitOk)
{
    // Done by TEST_SETUP
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, EnableInverter)
{
    VehicleControl_Status_T status = VehicleControl_EnableInverter(&mVehicleControl);
    TEST_ASSERT_EQUAL(status, VEHICLECONTORL_STATUS_OK);
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, DisableInverter)
{
    VehicleControl_Status_T status = VehicleControl_DisableInverter(&mVehicleControl);
    TEST_ASSERT_EQUAL(status, VEHICLECONTORL_STATUS_OK);
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, RequestMotorTorque)
{
    float torque = 0.0f;
    VehicleState_InverterDirection_T direction = VEHICLESTATE_INVERTER_FORWARD;
    VehicleControl_Status_T status = VehicleControl_RequestMotorTorque(&mVehicleControl, torque, direction);

    TEST_ASSERT_EQUAL(status, VEHICLECONTORL_STATUS_OK);
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, SetPowerChannel)
{
    uint8_t channel = 0;
    bool enabled = true;
    VehicleControl_Status_T status = VehicleControl_SetPowerChannel(&mVehicleControl, channel, enabled);
    
    TEST_ASSERT_EQUAL(status, VEHICLECONTORL_STATUS_OK);
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, SetECUError)
{
    bool error = true;
    VehicleControl_Status_T status = VehicleControl_SetECUError(&mVehicleControl, error);
    
    TEST_ASSERT_EQUAL(status, VEHICLECONTORL_STATUS_OK);
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, SetDash)
{
    bool ledOn = true;
    VehicleControl_Status_T status = VehicleControl_SetDash(&mVehicleControl, ledOn);
    
    TEST_ASSERT_EQUAL(status, VEHICLECONTORL_STATUS_OK);
}

TEST_GROUP_RUNNER(VEHICLEINTERFACE_VEHICLECONTROL)
{
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLECONTROL, InitOk);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLECONTROL, EnableInverter);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLECONTROL, DisableInverter);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLECONTROL, RequestMotorTorque);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLECONTROL, SetPowerChannel);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLECONTROL, SetECUError);
    RUN_TEST_CASE(VEHICLEINTERFACE_VEHICLECONTROL, SetDash);
}
