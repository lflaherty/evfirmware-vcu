/**
 * TestVehicleControl.c
 * 
 *  Created on: May 8 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>
#include <stdio.h>

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"

#include "time/tasktimer/MockTasktimer.h" // Needed for vehicle state
#include "lib/logging/MockLogging.h"
#include "device/inverter/MockCInverter.h"
#include "device/pdm/MockPdm.h"

// source code under test
#include "vehicleInterface/vehicleControl/vehicleControl.c"

// Pins
static GPIO_TypeDef testPinGpioBankA;
static GPIO_TypeDef testPinGpioBankB;
static uint16_t testPinBMSGpioPin = 1;
static uint16_t testPinBSPDGpioPin = 2;
static uint16_t testPinIMDGpioPin = 3;
static uint16_t testPinSDCOutGpioPin = 1;
static uint16_t testPinECUErrorGpioPin = 2;
static GPIO_T testPinBMSGpio;
static GPIO_T testPinBSPDGpio;
static GPIO_T testPinIMDGpio;
static GPIO_T testPinSDCOutGpio;
static GPIO_T testPinECUErrorGpio;

// Depdendant modules
static Logging_T testLog;
static VehicleState_T testVehicleState;
static SDC_Config_T sdcConfig;
static CInverter_T mInverter;
static PDM_T mPdm;

static VehicleControl_T mVehicleControl;

static void resetInputs(void)
{
    mockSet_GPIO_Asserted(testPinBMSGpio.GPIOx, testPinBMSGpio.GPIO_Pin, false);
    mockSet_GPIO_Asserted(testPinBSPDGpio.GPIOx, testPinBSPDGpio.GPIO_Pin, false);
    mockSet_GPIO_Asserted(testPinIMDGpio.GPIOx, testPinIMDGpio.GPIO_Pin, false);
    mockSet_GPIO_Asserted(testPinSDCOutGpio.GPIOx, testPinSDCOutGpio.GPIO_Pin, false);
    mockSet_GPIO_Asserted(testPinECUErrorGpio.GPIOx, testPinECUErrorGpio.GPIO_Pin, false);
}

static void registerGPIOs(void)
{
    testPinBMSGpio.GPIOx = &testPinGpioBankA;
    testPinBMSGpio.GPIO_Pin = testPinBMSGpioPin;
    mock_GPIO_RegisterPin(testPinBMSGpio.GPIOx, testPinBMSGpio.GPIO_Pin);

    testPinBSPDGpio.GPIOx = &testPinGpioBankA;
    testPinBSPDGpio.GPIO_Pin = testPinBSPDGpioPin;
    mock_GPIO_RegisterPin(testPinBSPDGpio.GPIOx, testPinBSPDGpio.GPIO_Pin);

    testPinIMDGpio.GPIOx = &testPinGpioBankA;
    testPinIMDGpio.GPIO_Pin = testPinIMDGpioPin;
    mock_GPIO_RegisterPin(testPinIMDGpio.GPIOx, testPinIMDGpio.GPIO_Pin);

    testPinSDCOutGpio.GPIOx = &testPinGpioBankB;
    testPinSDCOutGpio.GPIO_Pin = testPinSDCOutGpioPin;
    mock_GPIO_RegisterPin(testPinSDCOutGpio.GPIOx, testPinSDCOutGpio.GPIO_Pin);

    testPinECUErrorGpio.GPIOx = &testPinGpioBankB;
    testPinECUErrorGpio.GPIO_Pin = testPinECUErrorGpioPin;
    mock_GPIO_RegisterPin(testPinECUErrorGpio.GPIOx, testPinECUErrorGpio.GPIO_Pin);
}

TEST_GROUP(VEHICLEINTERFACE_VEHICLECONTROL);

TEST_SETUP(VEHICLEINTERFACE_VEHICLECONTROL)
{
    registerGPIOs();
    resetInputs();

    mockSemaphoreSetLocked(false);

    // Init logging & task timer
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);

    // Init vehicle state
    TEST_ASSERT_EQUAL(
        VEHICLESTATE_STATUS_OK,
        VehicleState_Init(&testLog, &testVehicleState));

    // Init SDC
    sdcConfig.state = &testVehicleState;
    sdcConfig.pinBMS = &testPinBMSGpio;
    sdcConfig.pinBSPD = &testPinBSPDGpio;
    sdcConfig.pinIMD = &testPinIMDGpio;
    sdcConfig.pinSDCOut = &testPinSDCOutGpio;
    sdcConfig.pinECUError = &testPinECUErrorGpio;
    TEST_ASSERT_EQUAL(SDC_STATUS_OK, SDC_Init(&testLog, &sdcConfig));
    mockLogClear();

    // Init mocks required for vehicle control
    TEST_ASSERT_EQUAL(CINVERTER_STATUS_OK, CInverter_Init(&testLog, &mInverter));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_Init(&testLog, &mPdm));

    memset(&mVehicleControl, 0, sizeof(VehicleControl_T));
    mVehicleControl.inverter = &mInverter;
    mVehicleControl.pdm = &mPdm;
    VehicleControl_Status_T status = VehicleControl_Init(&testLog, &mVehicleControl);
    TEST_ASSERT_EQUAL(VEHICLECONTROL_STATUS_OK, status);

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
    TEST_ASSERT_EQUAL(status, VEHICLECONTROL_STATUS_OK);
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, DisableInverter)
{
    VehicleControl_Status_T status = VehicleControl_DisableInverter(&mVehicleControl);
    TEST_ASSERT_EQUAL(status, VEHICLECONTROL_STATUS_OK);
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, RequestMotorTorque)
{
    float torque = 0.0f;
    VehicleState_InverterDirection_T direction = VEHICLESTATE_INVERTER_FORWARD;
    VehicleControl_Status_T status = VehicleControl_RequestMotorTorque(&mVehicleControl, torque, direction);

    TEST_ASSERT_EQUAL(status, VEHICLECONTROL_STATUS_OK);
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, SetPowerChannel)
{
    uint8_t channel = 0;
    bool enabled = true;
    VehicleControl_Status_T status = VehicleControl_SetPowerChannel(&mVehicleControl, channel, enabled);
    
    TEST_ASSERT_EQUAL(status, VEHICLECONTROL_STATUS_OK);
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, SetECUError)
{
    TEST_ASSERT_FALSE(mockGet_GPIO_Asserted(testPinECUErrorGpio.GPIOx, testPinECUErrorGpio.GPIO_Pin));

    // Assert error
    TEST_ASSERT_EQUAL(
        VEHICLECONTROL_STATUS_OK,
        VehicleControl_SetECUError(&mVehicleControl, true));
    TEST_ASSERT_TRUE(mockGet_GPIO_Asserted(testPinECUErrorGpio.GPIOx, testPinECUErrorGpio.GPIO_Pin));

    // Assert de-assert
    TEST_ASSERT_EQUAL(
        VEHICLECONTROL_STATUS_OK,
        VehicleControl_SetECUError(&mVehicleControl, false));
    TEST_ASSERT_FALSE(mockGet_GPIO_Asserted(testPinECUErrorGpio.GPIOx, testPinECUErrorGpio.GPIO_Pin));
}

TEST(VEHICLEINTERFACE_VEHICLECONTROL, SetDash)
{
    bool ledOn = true;
    VehicleControl_Status_T status = VehicleControl_SetDash(&mVehicleControl, ledOn);
    
    TEST_ASSERT_EQUAL(status, VEHICLECONTROL_STATUS_OK);
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

#define INVOKE_TEST VEHICLEINTERFACE_VEHICLECONTROL
#include "test_main.h"
