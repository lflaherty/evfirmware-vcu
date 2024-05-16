/*
 * TestTaskTimer.c
 *
 *  Created on 1 May 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "logging/MockLogging.h"

// source code under test
#include "tasktimer/tasktimer.c"

static Logging_T testLog;

static TIM_HandleTypeDef htim1;
static TIM_HandleTypeDef htimOther; // don't attach this to a task timer

// HAL interrupts:
// TODO move to mock header
void TaskTimer_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim);

TEST_GROUP(TIME_TASKTIMER);

TEST_SETUP(TIME_TASKTIMER)
{
    // Setup some mock data
    htim1.Instance = TIM1;
    htimOther.Instance = TIM2;

    // Rest notification counter
    mockSetTaskNotifyValue(0);

    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));

    TaskTimer_Status_T initStatus = TaskTimer_Init(
        &testLog,
        &htim1);
    TEST_ASSERT_EQUAL(TASKTIMER_STATUS_OK, initStatus);

    mockLogClear();
}

TEST_TEAR_DOWN(TIME_TASKTIMER)
{
    mockLogClear();
}

TEST(TIME_TASKTIMER, InitOk)
{
    // Done by TEST_SETUP
}

TEST(TIME_TASKTIMER, RegisterTask)
{
    TaskHandle_t someHandle;

    TEST_ASSERT_EQUAL(
        TASKTIMER_STATUS_OK,
        TaskTimer_RegisterTask(&someHandle, TASKTIMER_FREQUENCY_100HZ));
}

TEST(TIME_TASKTIMER, TimerElapsed)
{
    TaskHandle_t someHandle;
    TEST_ASSERT_EQUAL(
        TASKTIMER_STATUS_OK,
        TaskTimer_RegisterTask(&someHandle, TASKTIMER_FREQUENCY_100HZ));
    
    TEST_ASSERT_EQUAL(0, mockGetTaskNotifyValue());

    // This is the timer attached to 100Hz:
    TaskTimer_TIM_PeriodElapsedCallback(&htim1);
    TEST_ASSERT_EQUAL(1, mockGetTaskNotifyValue());

    // Another timer not used as a task timer shouldn't trigger anything
    TaskTimer_TIM_PeriodElapsedCallback(&htimOther);
    TEST_ASSERT_EQUAL(1, mockGetTaskNotifyValue());
}

TEST_GROUP_RUNNER(TIME_TASKTIMER)
{
    RUN_TEST_CASE(TIME_TASKTIMER, InitOk);
    RUN_TEST_CASE(TIME_TASKTIMER, RegisterTask);
    RUN_TEST_CASE(TIME_TASKTIMER, TimerElapsed);
}

#define INVOKE_TEST TIME_TASKTIMER
#include "test_main.h"
