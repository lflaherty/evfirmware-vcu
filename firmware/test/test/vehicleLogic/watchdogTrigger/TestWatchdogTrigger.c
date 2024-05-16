/**
 * TestWatchdogTrigger.c
 * 
 *  Created on: Jul 31 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "logging/MockLogging.h"
#include "tasktimer/MockTasktimer.h"

// source code under test
#include "vehicleLogic/watchdogTrigger/watchdogTrigger.c"

static Logging_T testLog;

static GPIO_TypeDef mLedGpioBank;
static uint16_t mLedGpioPin = 1;
static GPIO_T mLedGpio;

static WatchdogTrigger_T mWatchdogTrigger;

TEST_GROUP(VEHICLELOGIC_WATCHDOGTRIGGER);

TEST_SETUP(VEHICLELOGIC_WATCHDOGTRIGGER)
{
    mLedGpio.GPIOx = &mLedGpioBank;
    mLedGpio.GPIO_Pin = mLedGpioPin;

    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);

    mock_GPIO_RegisterPin(mLedGpio.GPIOx, mLedGpio.GPIO_Pin);
    
    memset(&mWatchdogTrigger, 0U, sizeof(WatchdogTrigger_T));
    mWatchdogTrigger.blinkLED = &mLedGpio;
    WatchdogTrigger_Status_T status = WatchdogTrigger_Init(&testLog, &mWatchdogTrigger);
    TEST_ASSERT(WATCHDOGTRIGGER_STATUS_OK == status);

    const char* expectedLogging =
        "WatchdogTrigger_Init begin\n"
        "WatchdogTrigger_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for coming tests
    mockLogClear();
}

TEST_TEAR_DOWN(VEHICLELOGIC_WATCHDOGTRIGGER)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(mWatchdogTrigger.mutex));
    mockLogClear();
}

TEST(VEHICLELOGIC_WATCHDOGTRIGGER, InitOk)
{
    // Done by TEST_SETUP
}

TEST(VEHICLELOGIC_WATCHDOGTRIGGER, InitTaskRegisterError)
{
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_ERROR_FULL);

    WatchdogTrigger_Status_T status = WatchdogTrigger_Init(&testLog, &mWatchdogTrigger);
    TEST_ASSERT(WATCHDOGTRIGGER_STATUS_ERROR_INIT == status);

    const char* expectedLogging =
        "WatchdogTrigger_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(VEHICLELOGIC_WATCHDOGTRIGGER, TestTask)
{
    bool expectedGpio = false;
    mockSet_GPIO_Asserted(mLedGpio.GPIOx, mLedGpio.GPIO_Pin, false);

    for (uint32_t i = 0; i < 10000; ++i) {
        mockSetTaskNotifyValue(1); // to wake up
        WatchdogTrigger(&mWatchdogTrigger);

        if ((i + 1) % 100 == 0) {
            expectedGpio = !expectedGpio;
        }
        TEST_ASSERT_EQUAL(expectedGpio, mockGet_GPIO_Asserted(mLedGpio.GPIOx, mLedGpio.GPIO_Pin));
    }
}

TEST_GROUP_RUNNER(VEHICLELOGIC_WATCHDOGTRIGGER)
{
    RUN_TEST_CASE(VEHICLELOGIC_WATCHDOGTRIGGER, InitOk);
    RUN_TEST_CASE(VEHICLELOGIC_WATCHDOGTRIGGER, InitTaskRegisterError);
    RUN_TEST_CASE(VEHICLELOGIC_WATCHDOGTRIGGER, TestTask);
}

#define INVOKE_TEST VEHICLELOGIC_WATCHDOGTRIGGER
#include "test_main.h"
