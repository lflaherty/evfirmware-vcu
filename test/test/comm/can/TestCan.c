/*
 * TestCan.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"

#include "lib/logging/logging.h"
#include "comm/can/can.h"
 
static Logging_T mLog;

TEST_GROUP(COMM_CAN);

TEST_SETUP(COMM_CAN)
{
    mLog.enableLogToDebug = true;
    mLog.enableLogToLogFile = false;
    mLog.enableLogToSerial = false;
    mockLogClear();
    mockSet_HAL_CAN_AllStatus(HAL_OK);
}

TEST_TEAR_DOWN(COMM_CAN)
{
    mockLogClear();
    mockSet_HAL_CAN_AllStatus(HAL_OK);
}

TEST(COMM_CAN, TestCanInitOk)
{
    CAN_Status_T status = CAN_Init(&mLog);

    TEST_ASSERT(CAN_STATUS_OK == status);

    const char* expectedLogging = 
        "CAN_Init begin\n"
        "CAN_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(COMM_CAN, TestCanConfigOk)
{
    CAN_HandleTypeDef handle = {0};
    CAN_Status_T status = CAN_Config(&handle);

    TEST_ASSERT(CAN_STATUS_OK == status);

    const char* expectedLogging = 
        "CAN_Config begin\n"
        "CAN_Config complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(COMM_CAN, TestCanConfigErrorCfg)
{
    CAN_HandleTypeDef handle = {0};
    mockSet_HAL_CAN_ConfigFilter_Status(HAL_ERROR);
    CAN_Status_T status = CAN_Config(&handle);

    TEST_ASSERT(CAN_STATUS_ERROR_CFG_FILTER == status);

    const char* expectedLogging = "CAN_Config begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(COMM_CAN, TestCanConfigErrorStart)
{
    CAN_HandleTypeDef handle = {0};
    mockSet_HAL_CAN_Start_Status(HAL_ERROR);
    CAN_Status_T status = CAN_Config(&handle);

    TEST_ASSERT(CAN_STATUS_ERROR_START == status);

    const char* expectedLogging = "CAN_Config begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(COMM_CAN, TestCanConfigErrorActivateNotification)
{
    CAN_HandleTypeDef handle = {0};
    mockSet_HAL_CAN_ActivateNotification_Status(HAL_ERROR);
    CAN_Status_T status = CAN_Config(&handle);

    TEST_ASSERT(CAN_STATUS_ERROR_START_NOTIFY == status);

    const char* expectedLogging = "CAN_Config begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

static void RunAllTests(void)
{
    RUN_TEST_CASE(COMM_CAN, TestCanInitOk);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigOk);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigErrorCfg);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigErrorStart);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigErrorActivateNotification);
}

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, RunAllTests);
}
