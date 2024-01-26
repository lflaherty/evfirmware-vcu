/**
 * TestWheelspeed.c
 * 
 *  Created on: 28 Jan 2022
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
#include "device/wheelspeed/wheelspeed.c"

// Pins
static GPIO_TypeDef testPinGpioBankA;
static uint16_t testPinFrontWsGpioPin = 1;
static uint16_t testPinRearWsGpioPin = 2;
static GPIO_T testPinFrontWsGpio;
static GPIO_T testPinRearWsGpio;

static TIM_HandleTypeDef htim2;

// Modules
static Logging_T testLog;
static VehicleState_T testVehicleState;
static Wheelspeed_Config_T testConfig;

static const uint16_t SENSOR_TEETH = 25U;

static void resetInputs(void)
{
    mockSet_GPIO_Asserted(testPinFrontWsGpio.GPIOx, testPinFrontWsGpio.GPIO_Pin, false);
    mockSet_GPIO_Asserted(testPinRearWsGpio.GPIOx, testPinRearWsGpio.GPIO_Pin, false);
}

static void registerGPIOs(void)
{
    testPinFrontWsGpio.GPIOx = &testPinGpioBankA;
    testPinFrontWsGpio.GPIO_Pin = testPinFrontWsGpioPin;
    mock_GPIO_RegisterPin(testPinFrontWsGpio.GPIOx, testPinFrontWsGpio.GPIO_Pin);

    testPinRearWsGpio.GPIOx = &testPinGpioBankA;
    testPinRearWsGpio.GPIO_Pin = testPinRearWsGpioPin;
    mock_GPIO_RegisterPin(testPinRearWsGpio.GPIOx, testPinRearWsGpio.GPIO_Pin);
}

TEST_GROUP(DEVICE_WHEELSPEED);

TEST_SETUP(DEVICE_WHEELSPEED)
{
    registerGPIOs();
    resetInputs();
    htim2.Instance = TIM2;

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

    // Init wheelspeed
    testConfig.state = &testVehicleState;
    testConfig.logging = &testLog;
    testConfig.frontWsPin = &testPinFrontWsGpio;
    testConfig.rearWsPin = &testPinRearWsGpio;
    testConfig.timerInstance = htim2.Instance;
    testConfig.sensorTeeth = SENSOR_TEETH;
    Wheelspeed_Status_T status = Wheelspeed_Init(&testConfig);
    TEST_ASSERT_EQUAL(WHEELSPEED_STATUS_OK, status);

    const char* expectedLogging =
        "Wheelspeed_Init begin\n"
        "Wheelspeed_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    TEST_ASSERT_EQUAL(0U, testVehicleState.data.vehicle.wheelspeed.wheelspeedFront);
    TEST_ASSERT_EQUAL(0U, testVehicleState.data.vehicle.wheelspeed.wheelspeedRear);
    TEST_ASSERT_EQUAL(0U, testVehicleState.data.vehicle.wheelspeed.wssCountFront);
    TEST_ASSERT_EQUAL(0U, testVehicleState.data.vehicle.wheelspeed.wssCountRear);
}

TEST_TEAR_DOWN(DEVICE_WHEELSPEED)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(testVehicleState.mutex));
    mockClearStreamBufferData(mSampleStream.sampleStreamHandle);
}

TEST(DEVICE_WHEELSPEED, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_WHEELSPEED, ZeroTransitions)
{
    uint8_t samples[WSS_SAMPLES_PER_SECOND] = { 0U };

    for (uint16_t i = 0; i < WSS_SAMPLES_PER_SECOND; ++i) {
        mockSet_GPIO_Asserted(testPinFrontWsGpio.GPIOx, testPinFrontWsGpio.GPIO_Pin, samples[i]);
        mockSet_GPIO_Asserted(testPinRearWsGpio.GPIOx, testPinRearWsGpio.GPIO_Pin, samples[i]);

        Wheelspeed_TIM_IRQHandler(&htim2);
    }

    mockSetTaskNotifyValue(1); // to wake up
    Wheelspeed_TaskMethod();

    TEST_ASSERT_EQUAL(0U, testVehicleState.data.vehicle.wheelspeed.wheelspeedFront);
    TEST_ASSERT_EQUAL(0U, testVehicleState.data.vehicle.wheelspeed.wheelspeedRear);
    TEST_ASSERT_EQUAL(0U, testVehicleState.data.vehicle.wheelspeed.wssCountFront);
    TEST_ASSERT_EQUAL(0U, testVehicleState.data.vehicle.wheelspeed.wssCountRear);
}

TEST(DEVICE_WHEELSPEED, MultipleTransitions)
{
    // Setup test data
    uint8_t samplesFront[WSS_SAMPLES_PER_SECOND] = { 0U };
    samplesFront[5] = 1U;
    samplesFront[6] = 1U;
    samplesFront[100] = 1U;
    uint8_t samplesRear[WSS_SAMPLES_PER_SECOND] = { 0U };
    samplesRear[420] = 1U;

    for (uint16_t i = 0; i < WSS_SAMPLES_PER_SECOND; ++i) {
        mockSet_GPIO_Asserted(testPinFrontWsGpio.GPIOx, testPinFrontWsGpio.GPIO_Pin, samplesFront[i]);
        mockSet_GPIO_Asserted(testPinRearWsGpio.GPIOx, testPinRearWsGpio.GPIO_Pin, samplesRear[i]);

        Wheelspeed_TIM_IRQHandler(&htim2);
    }

    mockSetTaskNotifyValue(1); // to wake up
    Wheelspeed_TaskMethod();

    TEST_ASSERT_EQUAL(4, testVehicleState.data.vehicle.wheelspeed.wheelspeedFront);
    TEST_ASSERT_EQUAL(2, testVehicleState.data.vehicle.wheelspeed.wheelspeedRear);
    TEST_ASSERT_EQUAL(2, testVehicleState.data.vehicle.wheelspeed.wssCountFront);
    TEST_ASSERT_EQUAL(1, testVehicleState.data.vehicle.wheelspeed.wssCountRear);
}

TEST(DEVICE_WHEELSPEED, HighRPM)
{
    // Setup test data
    // To create ~1400 RPM, need 584 transitions
    // To create ~2000 RPM, need 834 transitions
    _Static_assert(2*584 < WSS_SAMPLES_PER_SECOND, "not enough samples");
    _Static_assert(2*834 < WSS_SAMPLES_PER_SECOND, "not enough samples");
    uint8_t samples1400Rpm[WSS_SAMPLES_PER_SECOND] = { 0U };
    uint8_t samples2000Rpm[WSS_SAMPLES_PER_SECOND] = { 0U };
    for (uint16_t i = 1; i < 2*584; i += 2) {
        samples1400Rpm[i] = 1U;
    }
    for (uint16_t i = 1; i < 2*834; i += 2) {
        samples2000Rpm[i] = 1U;
    }

    for (uint16_t i = 0; i < WSS_SAMPLES_PER_SECOND; ++i) {
        mockSet_GPIO_Asserted(testPinFrontWsGpio.GPIOx, testPinFrontWsGpio.GPIO_Pin, samples1400Rpm[i]);
        mockSet_GPIO_Asserted(testPinRearWsGpio.GPIOx, testPinRearWsGpio.GPIO_Pin, samples1400Rpm[i]);

        Wheelspeed_TIM_IRQHandler(&htim2);
    }

    mockSetTaskNotifyValue(1); // to wake up
    Wheelspeed_TaskMethod();

    TEST_ASSERT_EQUAL(1401, testVehicleState.data.vehicle.wheelspeed.wheelspeedFront);
    TEST_ASSERT_EQUAL(1401, testVehicleState.data.vehicle.wheelspeed.wheelspeedRear);
    TEST_ASSERT_EQUAL(584, testVehicleState.data.vehicle.wheelspeed.wssCountFront);
    TEST_ASSERT_EQUAL(584, testVehicleState.data.vehicle.wheelspeed.wssCountRear);


    // Now increase the speed to 2000rpm
    for (uint16_t i = 0; i < WSS_SAMPLES_PER_SECOND; ++i) {
        mockSet_GPIO_Asserted(testPinFrontWsGpio.GPIOx, testPinFrontWsGpio.GPIO_Pin, samples2000Rpm[i]);
        mockSet_GPIO_Asserted(testPinRearWsGpio.GPIOx, testPinRearWsGpio.GPIO_Pin, samples2000Rpm[i]);

        Wheelspeed_TIM_IRQHandler(&htim2);
    }

    // Need to spin the task 100 times since it's a 1Hz task on a 100Hz timer
    for (uint16_t i = 0; i < 100U; ++i) {
        mockSetTaskNotifyValue(1); // to wake up
        Wheelspeed_TaskMethod();
    }

    TEST_ASSERT_EQUAL(2001, testVehicleState.data.vehicle.wheelspeed.wheelspeedFront);
    TEST_ASSERT_EQUAL(2001, testVehicleState.data.vehicle.wheelspeed.wheelspeedRear);
    TEST_ASSERT_EQUAL(834, testVehicleState.data.vehicle.wheelspeed.wssCountFront);
    TEST_ASSERT_EQUAL(834, testVehicleState.data.vehicle.wheelspeed.wssCountRear);

}

TEST_GROUP_RUNNER(DEVICE_WHEELSPEED)
{
    RUN_TEST_CASE(DEVICE_WHEELSPEED, InitOk);
    RUN_TEST_CASE(DEVICE_WHEELSPEED, ZeroTransitions);
    RUN_TEST_CASE(DEVICE_WHEELSPEED, MultipleTransitions);
    RUN_TEST_CASE(DEVICE_WHEELSPEED, HighRPM);
}

#define INVOKE_TEST DEVICE_WHEELSPEED
#include "test_main.h"
