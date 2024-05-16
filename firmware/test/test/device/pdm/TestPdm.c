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
#include "logging/MockLogging.h"
#include "tasktimer/MockTasktimer.h"

// source code under test
#include "device/pdm/pdm.c"

// Pins
static GPIO_TypeDef gpioBankA;
static GPIO_TypeDef gpioBankB;
static GPIO_T pinCh1 = { .GPIOx = &gpioBankA, .GPIO_Pin = 1U };
static GPIO_T pinCh2 = { .GPIOx = &gpioBankA, .GPIO_Pin = 2U };
static GPIO_T pinCh3 = { .GPIOx = &gpioBankA, .GPIO_Pin = 3U };
static GPIO_T pinCh4 = { .GPIOx = &gpioBankA, .GPIO_Pin = 4U };
static GPIO_T pinCh5 = { .GPIOx = &gpioBankB, .GPIO_Pin = 1U };
static GPIO_T pinCh6 = { .GPIOx = &gpioBankB, .GPIO_Pin = 2U };

enum PDM_Channels {
    Ch1,
    Ch2,
    Ch3,
    Ch4,
    Ch5,
    Ch6,
    Num_Channels
};
static PDM_Channel_T channels[] = {
    [Ch1] = { .pin = &pinCh1 },
    [Ch2] = { .pin = &pinCh2 },
    [Ch3] = { .pin = &pinCh3 },
    [Ch4] = { .pin = &pinCh4 },
    [Ch5] = { .pin = &pinCh5 },
    [Ch6] = { .pin = &pinCh6 },
};
_Static_assert(Num_Channels == sizeof(channels) / sizeof(PDM_Channel_T), "channel config incorrect");

// Modules
static VehicleState_T mVehicleState;
static Logging_T testLog;
static PDM_T mPdm;

static void registerGPIOs(void)
{
    mock_GPIO_RegisterPin(pinCh1.GPIOx, pinCh1.GPIO_Pin);
    mock_GPIO_RegisterPin(pinCh2.GPIOx, pinCh2.GPIO_Pin);
    mock_GPIO_RegisterPin(pinCh3.GPIOx, pinCh3.GPIO_Pin);
    mock_GPIO_RegisterPin(pinCh4.GPIOx, pinCh4.GPIO_Pin);
    mock_GPIO_RegisterPin(pinCh5.GPIOx, pinCh5.GPIO_Pin);
    mock_GPIO_RegisterPin(pinCh6.GPIOx, pinCh6.GPIO_Pin);
}

static void resetGPIOs(void)
{
    GPIO_WritePin(&pinCh1, false);
    GPIO_WritePin(&pinCh2, false);
    GPIO_WritePin(&pinCh3, false);
    GPIO_WritePin(&pinCh4, false);
    GPIO_WritePin(&pinCh5, false);
    GPIO_WritePin(&pinCh6, false);
}

TEST_GROUP(DEVICE_PDM);

TEST_SETUP(DEVICE_PDM)
{
    registerGPIOs();
    resetGPIOs();

    // Init logging
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));

    // Init vehicle state
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);
    TEST_ASSERT_EQUAL(
        VEHICLESTATE_STATUS_OK,
        VehicleState_Init(&testLog, &mVehicleState));
    mockLogClear();
    
    // Init PDM
    mPdm.channels = channels;
    mPdm.numChannels = Num_Channels;
    mPdm.vehicleState = &mVehicleState;
    PDM_Status_T status = PDM_Init(&testLog, &mPdm);
    TEST_ASSERT(PDM_STATUS_OK == status);

    const char* expectedLogging =
        "PDM_Init begin\n"
        "PDM_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // check GPIOs are still low
    TEST_ASSERT_FALSE(GPIO_ReadPin(&pinCh1));
    TEST_ASSERT_FALSE(GPIO_ReadPin(&pinCh2));
    TEST_ASSERT_FALSE(GPIO_ReadPin(&pinCh3));
    TEST_ASSERT_FALSE(GPIO_ReadPin(&pinCh4));
    TEST_ASSERT_FALSE(GPIO_ReadPin(&pinCh5));
    TEST_ASSERT_FALSE(GPIO_ReadPin(&pinCh6));
}

TEST_TEAR_DOWN(DEVICE_PDM)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(mVehicleState.mutex));
}

TEST(DEVICE_PDM, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_PDM, NoChannels)
{
    PDM_T altPdm; // just register no channels
    memset(&altPdm, 0, sizeof(altPdm));
    altPdm.vehicleState = &mVehicleState;

    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_Init(&testLog, &altPdm));
    TEST_ASSERT_EQUAL(NULL, altPdm.channels);
    TEST_ASSERT_EQUAL(0, altPdm.numChannels);
    TEST_ASSERT_EQUAL(
        PDM_STATUS_ERROR_INVALID_CH,
        PDM_SetOutputEnabled(&altPdm, 0, true));
}

TEST(DEVICE_PDM, IncorrectConfig)
{
    PDM_T altPdm; // just register no channels
    memset(&altPdm, 0, sizeof(altPdm));
    altPdm.vehicleState = &mVehicleState;

    PDM_Channel_T altChannels[] = {
        [0] = { .pin = &pinCh1 },
    };
    size_t altNumChannels = sizeof(altChannels) / sizeof(PDM_Channel_T);

    altPdm.channels = altChannels;
    // leave numChannels empty
    TEST_ASSERT_EQUAL(PDM_STATUS_ERROR_CONFIG, PDM_Init(&testLog, &altPdm));

    // now do the other way around
    memset(&altPdm, 0, sizeof(altPdm));
    altPdm.numChannels = altNumChannels;
    altPdm.vehicleState = &mVehicleState;
    TEST_ASSERT_EQUAL(PDM_STATUS_ERROR_CONFIG, PDM_Init(&testLog, &altPdm));
}

TEST(DEVICE_PDM, ControlChannels)
{
    // Setting all to true
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch1, true));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch2, true));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch3, true));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch4, true));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch5, true));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch6, true));
    for (uint8_t i = 0; i < Num_Channels; ++i) {
        TEST_ASSERT_TRUE(GPIO_ReadPin(channels[i].pin));
        TEST_ASSERT_TRUE(mVehicleState.data.glv.pdmChState[i]);
    }

    // Setting all to false
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch1, false));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch2, false));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch3, false));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch4, false));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch5, false));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, Ch6, false));
    for (uint8_t i = 0; i < Num_Channels; ++i) {
        TEST_ASSERT_FALSE(GPIO_ReadPin(channels[i].pin));
        TEST_ASSERT_FALSE(mVehicleState.data.glv.pdmChState[i]);
    }

    // Iterate through controlling all channels individually
    for (uint8_t i = 0; i < Num_Channels; ++i) {
        resetGPIOs();
        memset(mVehicleState.data.glv.pdmChState, 0, sizeof(mVehicleState.data.glv.pdmChState) / sizeof(bool));

        TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_SetOutputEnabled(&mPdm, i, true));

        TEST_ASSERT_TRUE(GPIO_ReadPin(channels[i].pin));
        TEST_ASSERT_TRUE(mVehicleState.data.glv.pdmChState[i]);
        for (uint8_t j = 0; j < Num_Channels; ++j) {
            if (i == j) // skip the pin currently being tested
                continue;
            TEST_ASSERT_FALSE(GPIO_ReadPin(channels[j].pin));
            TEST_ASSERT_FALSE(mVehicleState.data.glv.pdmChState[j]);
        }
    }

    // Invalid channel
    TEST_ASSERT_EQUAL(PDM_STATUS_ERROR_INVALID_CH, PDM_SetOutputEnabled(&mPdm, 10, true));
}

TEST_GROUP_RUNNER(DEVICE_PDM)
{
    RUN_TEST_CASE(DEVICE_PDM, InitOk);
    RUN_TEST_CASE(DEVICE_PDM, NoChannels);
    RUN_TEST_CASE(DEVICE_PDM, IncorrectConfig);
    RUN_TEST_CASE(DEVICE_PDM, ControlChannels);
}

#define INVOKE_TEST DEVICE_PDM
#include "test_main.h"
