/**
 * TestDiscreteSense.c
 * 
 *  Created on: 21 May 2023
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

#include "time/tasktimer/MockTasktimer.h"
#include "lib/logging/MockLogging.h"
#include "io/adc/adc.h"
#include "vehicleInterface/vehicleState/vehicleState.h"

// source code under test
#include "device/discretesense/discretesense.c"

// Pins and ADC channels
static GPIO_TypeDef testPinGpioBankA;
static const uint16_t testPinDashButtonGpioPin = 1;
static GPIO_T testPinDashButtonGpio = {
    .GPIOx = &testPinGpioBankA,
    .GPIO_Pin = testPinDashButtonGpioPin
};
static const ADC_Channel_T adcChAccelPedalA = ADC_CONVERSIONCHANNEL0;
static const ADC_Channel_T adcChAccelPedalB = ADC_CONVERSIONCHANNEL1;
static const ADC_Channel_T adcChBrakePedalFront = ADC_CONVERSIONCHANNEL2;
static const ADC_Channel_T adcChBrakePedalRear = ADC_CONVERSIONCHANNEL3;
#define NUM_CHANNELS 4

static ADC_HandleTypeDef hadc1 = {
    .Instance = ADC1,
    .Init = (ADC_InitTypeDef){
        .ContinuousConvMode = ENABLE,
        .DataAlign = ADC_DATAALIGN_RIGHT,
        .NbrOfConversion = NUM_CHANNELS,
    }
};

// Modules
static Logging_T testLog;
static VehicleState_T testVehicleState;
static ADC_Config_T testADCConfig = {
    .logger = &testLog,
    .handle = &hadc1,
    .adcIrq = DMA2_Stream0_IRQn,
    .numChannelsUsed = NUM_CHANNELS,
};
static DiscreteSense_T testDiscreteSense = {
    // Required module refs
    .logger = &testLog,
    .state = &testVehicleState,
    // Config
    .adcAccelPedalA = adcChAccelPedalA,
    .adcAccelPedalB = adcChAccelPedalB,
    .adcBrakeFront = adcChBrakePedalFront,
    .adcBrakeRear = adcChBrakePedalRear,
    .gpioDashboardButton = &testPinDashButtonGpio,
    .scalingAccelPedalA = {
        .lowerScaling = 0,
        .upperScaling = 1000,
        .saturate = true,
    },
    .scalingAccelPedalB = {
        .lowerScaling = 1000,
        .upperScaling = 2000,
        .saturate = true,
    },
    .scalingBrakeFront = {
        .lowerScaling = 2000,
        .upperScaling = 3000,
        .saturate = true,
    },
    .scalingBrakeRear = {
        .lowerScaling = 3000,
        .upperScaling = 4000,
        .saturate = true,
    },
};

// HAL interrupts:
// TODO move these to mock
extern void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
extern void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

static void resetInputs(void)
{
    mockSet_GPIO_Asserted(testPinDashButtonGpio.GPIOx, testPinDashButtonGpio.GPIO_Pin, false);
}

static void registerGPIOs(void)
{
    mock_GPIO_RegisterPin(testPinDashButtonGpio.GPIOx, testPinDashButtonGpio.GPIO_Pin);
}

static void expectUnset(void)
{
    TEST_ASSERT_EQUAL_FLOAT(0.0f, testVehicleState.data.inputs.accel);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, testVehicleState.data.inputs.accelA);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, testVehicleState.data.inputs.accelB);
    TEST_ASSERT_EQUAL(0, testVehicleState.data.inputs.accelRawA);
    TEST_ASSERT_EQUAL(0, testVehicleState.data.inputs.accelRawB);
    TEST_ASSERT_FALSE(testVehicleState.data.inputs.accelValid);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, testVehicleState.data.inputs.brakePresFront);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, testVehicleState.data.inputs.brakePresRear);
    TEST_ASSERT_EQUAL(0, testVehicleState.data.inputs.brakeRawFront);
    TEST_ASSERT_EQUAL(0, testVehicleState.data.inputs.brakeRawRear);
}

TEST_GROUP(DEVICE_DISCRETESENSE);

TEST_SETUP(DEVICE_DISCRETESENSE)
{
    registerGPIOs();
    resetInputs();

    // Init logging & task timer
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);

    // Init ADC
    TEST_ASSERT_EQUAL(ADC_STATUS_OK, ADC_Init(&testADCConfig));

    // Init vehicle state
    TEST_ASSERT_EQUAL(
        VEHICLESTATE_STATUS_OK,
        VehicleState_Init(&testLog, &testVehicleState));

    mockLogClear();

    // Init wheelspeed
    DiscreteSense_Status_T status = DiscreteSense_Init(&testDiscreteSense);
    TEST_ASSERT_EQUAL(DISCRETESENSE_STATUS_OK, status);

    const char* expectedLogging =
        "DiscreteSense_Init begin\n"
        "DiscreteSense_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    expectUnset();
}

TEST_TEAR_DOWN(DEVICE_DISCRETESENSE)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(testVehicleState.mutex));
}

TEST(DEVICE_DISCRETESENSE, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_DISCRETESENSE, UpdateNormal)
{
    // No updates until timer elapsed
    for (uint16_t i = 0; i < 1000; ++i) {
        DiscreteSense_TaskMethod(&testDiscreteSense);
        expectUnset();
    }

    // set some values
    mockSetADCDataChannel(adcChAccelPedalA, 10);
    mockSetADCDataChannel(adcChAccelPedalB, 1020);
    mockSetADCDataChannel(adcChBrakePedalFront, 2100);
    mockSetADCDataChannel(adcChBrakePedalRear, 3120);

    // Run ADC callbacks
    HAL_ADC_ConvHalfCpltCallback(&hadc1);
    HAL_ADC_ConvCpltCallback(&hadc1);

    mockSetTaskNotifyValue(1); // to wake up
    DiscreteSense_TaskMethod(&testDiscreteSense);

    TEST_ASSERT_EQUAL(10, testVehicleState.data.inputs.accelRawA);
    TEST_ASSERT_EQUAL(1020, testVehicleState.data.inputs.accelRawB);
    TEST_ASSERT_EQUAL_FLOAT(0.01f, testVehicleState.data.inputs.accelA);
    TEST_ASSERT_EQUAL_FLOAT(0.02f, testVehicleState.data.inputs.accelB);
    TEST_ASSERT_EQUAL_FLOAT(0.015f, testVehicleState.data.inputs.accel);
    TEST_ASSERT_TRUE(testVehicleState.data.inputs.accelValid);
    TEST_ASSERT_EQUAL(2100, testVehicleState.data.inputs.brakeRawFront);
    TEST_ASSERT_EQUAL(3120, testVehicleState.data.inputs.brakeRawRear);
    TEST_ASSERT_EQUAL_FLOAT(0.1f, testVehicleState.data.inputs.brakePresFront);
    TEST_ASSERT_EQUAL_FLOAT(0.12f, testVehicleState.data.inputs.brakePresRear);
}

TEST_GROUP_RUNNER(DEVICE_DISCRETESENSE)
{
    RUN_TEST_CASE(DEVICE_DISCRETESENSE, InitOk);
    RUN_TEST_CASE(DEVICE_DISCRETESENSE, UpdateNormal);
}

#define INVOKE_TEST DEVICE_DISCRETESENSE
#include "test_main.h"
