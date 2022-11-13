/**
 * TestSDC.c
 * 
 *  Created on: Nov 13 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semaphore.h"

#include "time/tasktimer/MockTasktimer.h" // Needed for vehicle state
#include "lib/logging/MockLogging.h"

#include "vehicleInterface/vehicleState/vehicleState.h"

// source code under test
#include "device/sdc/sdc.c"

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

// Modules
static Logging_T testLog;
static VehicleState_T testVehicleState;
static SDC_Config_T testConfig;

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

TEST_GROUP(DEVICE_SDC);

TEST_SETUP(DEVICE_SDC)
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
    
    mockLogClear();
    
    // Init SDC
    testConfig.state = &testVehicleState;
    testConfig.pinBMS = &testPinBMSGpio;
    testConfig.pinBSPD = &testPinBSPDGpio;
    testConfig.pinIMD = &testPinIMDGpio;
    testConfig.pinSDCOut = &testPinSDCOutGpio;
    testConfig.pinECUError = &testPinECUErrorGpio;
    SDC_Status_T status = SDC_Init(&testLog, &testConfig);
    TEST_ASSERT(SDC_STATUS_OK == status);

    const char* expectedLogging =
        "SDC_Init begin\n"
        "SDC_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.bms);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.bspd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.imd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.out);
}

TEST_TEAR_DOWN(DEVICE_SDC)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());
    TEST_ASSERT_EQUAL(SDC_STATUS_OK, SDC_AssertECUFault(false));
}

TEST(DEVICE_SDC, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_SDC, NoUpdate)
{
    SDC_IRQHandler(0);
    SDC_TaskMethod();

    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.bms);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.bspd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.imd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.out);
}

TEST(DEVICE_SDC, Update1)
{
    mockSet_GPIO_Asserted(testPinBMSGpio.GPIOx, testPinBMSGpio.GPIO_Pin, true);

    SDC_IRQHandler(0);
    SDC_TaskMethod();

    TEST_ASSERT_TRUE(testVehicleState.data.vehicle.sdc.bms);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.bspd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.imd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.out);
}

TEST(DEVICE_SDC, Update2Separate)
{
    // Update pins separately
    mockSet_GPIO_Asserted(testPinBMSGpio.GPIOx, testPinBMSGpio.GPIO_Pin, true);
    SDC_IRQHandler(0);
    SDC_TaskMethod();

    TEST_ASSERT_TRUE(testVehicleState.data.vehicle.sdc.bms);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.bspd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.imd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.out);

    mockSet_GPIO_Asserted(testPinSDCOutGpio.GPIOx, testPinSDCOutGpio.GPIO_Pin, true);
    SDC_IRQHandler(0);
    SDC_TaskMethod();

    TEST_ASSERT_TRUE(testVehicleState.data.vehicle.sdc.bms);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.bspd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.imd);
    TEST_ASSERT_TRUE(testVehicleState.data.vehicle.sdc.out);
}

TEST(DEVICE_SDC, Update2Interleaved)
{
    // Update pins together, before RTOS task runs
    mockSet_GPIO_Asserted(testPinBMSGpio.GPIOx, testPinBMSGpio.GPIO_Pin, true);
    SDC_IRQHandler(0);

    mockSet_GPIO_Asserted(testPinSDCOutGpio.GPIOx, testPinSDCOutGpio.GPIO_Pin, true);
    SDC_IRQHandler(0);

    SDC_TaskMethod();

    TEST_ASSERT_TRUE(testVehicleState.data.vehicle.sdc.bms);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.bspd);
    TEST_ASSERT_FALSE(testVehicleState.data.vehicle.sdc.imd);
    TEST_ASSERT_TRUE(testVehicleState.data.vehicle.sdc.out);
}

TEST(DEVICE_SDC, AssertECU)
{
    TEST_ASSERT_FALSE(mockGet_GPIO_Asserted(testPinECUErrorGpio.GPIOx, testPinECUErrorGpio.GPIO_Pin));

    TEST_ASSERT_EQUAL(SDC_STATUS_OK, SDC_AssertECUFault(true));

    TEST_ASSERT_TRUE(mockGet_GPIO_Asserted(testPinECUErrorGpio.GPIOx, testPinECUErrorGpio.GPIO_Pin));
}

TEST_GROUP_RUNNER(DEVICE_SDC)
{
    RUN_TEST_CASE(DEVICE_SDC, InitOk);
    RUN_TEST_CASE(DEVICE_SDC, NoUpdate);
    RUN_TEST_CASE(DEVICE_SDC, Update1);
    RUN_TEST_CASE(DEVICE_SDC, Update2Separate);
    RUN_TEST_CASE(DEVICE_SDC, Update2Interleaved);
    RUN_TEST_CASE(DEVICE_SDC, AssertECU);
}

#define INVOKE_TEST DEVICE_SDC
#include "test_main.h"
