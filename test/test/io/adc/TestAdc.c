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
#include "io/adc/adc.h"

#include <stdio.h>
 
static Logging_T mLog;

TEST_GROUP(IO_ADC);

TEST_SETUP(IO_ADC)
{
    mLog.enableLogToDebug = true;
    mLog.enableLogToLogFile = false;
    mLog.enableLogToSerial = false;
    mockLogClear();
    // mockSet_HAL_CAN_AllStatus(HAL_OK);
}

TEST_TEAR_DOWN(IO_ADC)
{
    mockLogClear();
    // mockSet_HAL_CAN_AllStatus(HAL_OK);
}

TEST(IO_ADC, TestAdcInitOk)
{
    ADC_Status_T status = ADC_Init(&mLog, 1, 1);

    TEST_ASSERT(ADC_STATUS_OK == status);

    const char* expectedLogging = 
        "ADC_Init begin\n"
        "ADC_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

static void RunAllTests(void)
{
    RUN_TEST_CASE(IO_ADC, TestAdcInitOk);
}

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, RunAllTests);
}
