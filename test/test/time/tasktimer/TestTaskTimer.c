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

#include "lib/logging/MockLogging.h"

// source code under test
#include "time/tasktimer/tasktimer.c"

static Logging_T testLog;

// HAL interrupts:
// TODO move to mock header
void TaskTimer_TIM_PeriodElapsedCallback(TIM_HandleTypeDef* htim);

TEST_GROUP(TIME_TASKTIMER);

TEST_SETUP(TIME_TASKTIMER)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
}

TEST_TEAR_DOWN(TIME_TASKTIMER)
{
    mockLogClear();
}
TEST(TIME_TASKTIMER, TestTaskTimerInitOk)
{

}

TEST_GROUP_RUNNER(TIME_TASKTIMER)
{
    RUN_TEST_CASE(TIME_TASKTIMER, TestTaskTimerInitOk);
}