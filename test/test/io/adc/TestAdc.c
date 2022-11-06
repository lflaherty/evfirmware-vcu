/*
 * TestCan.c
 *
 *  Created on: 23 Aug 2021
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
#include "io/adc/adc.c"

static Logging_T mLog;

// HAL interrupts:
extern void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
extern void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

TEST_GROUP(IO_ADC);

TEST_SETUP(IO_ADC)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&mLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&mLog));
    mockLogClear();
    mockSet_HAL_ADC_Start_DMA_Status(HAL_OK);
}

TEST_TEAR_DOWN(IO_ADC)
{
    mockLogClear();
    mockSet_HAL_ADC_Start_DMA_Status(HAL_OK);
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

TEST(IO_ADC, TestAdcInitTooManyChannels)
{
    ADC_Status_T status = ADC_Init(&mLog, ADC_MAX_NUM_CHANNELS+1, 1);

    TEST_ASSERT(ADC_STATUS_ERROR_CHANNEL_COUNT == status);

    const char* expectedLogging = 
        "ADC_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(IO_ADC, TestAdcConfigOk)
{
    // Perform init
    ADC_Status_T status = ADC_Init(&mLog, ADC_MAX_NUM_CHANNELS, 1);
    TEST_ASSERT(ADC_STATUS_OK == status);

    // Perform config
    ADC_HandleTypeDef hadc;
    status = ADC_Config(&hadc);
    TEST_ASSERT(ADC_STATUS_OK == status);

    const char* expectedLogging = 
        "ADC_Init begin\n"
        "ADC_Init complete\n"
        "ADC_Config begin\n"
        "ADC_Config complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(IO_ADC, TestAdcConfigDmaError)
{
    // Perform init
    ADC_Status_T status = ADC_Init(&mLog, ADC_MAX_NUM_CHANNELS, 1);
    TEST_ASSERT(ADC_STATUS_OK == status);

    // Perform config
    ADC_HandleTypeDef hadc;
    mockSet_HAL_ADC_Start_DMA_Status(HAL_ERROR);
    status = ADC_Config(&hadc);
    TEST_ASSERT(ADC_STATUS_ERROR_DMA == status);

    const char* expectedLogging = 
        "ADC_Init begin\n"
        "ADC_Init complete\n"
        "ADC_Config begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(IO_ADC, TestAdcGetZero)
{
    // Perform init
    ADC_Status_T status = ADC_Init(&mLog, ADC_MAX_NUM_CHANNELS, 1);
    TEST_ASSERT(ADC_STATUS_OK == status);
    const char* expectedLogging = 
        "ADC_Init begin\n"
        "ADC_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // Attempt to get
    for (uint16_t i = 0; i < ADC_MAX_NUM_CHANNELS; ++i) {
        uint16_t adcVal = ADC_Get(i);
        TEST_ASSERT(0 == adcVal);
    }

    // Get invalid value
    TEST_ASSERT(ADC_INVALID == ADC_Get(ADC_MAX_NUM_CHANNELS+1));
}

TEST(IO_ADC, TestAdcInterruptHalf)
{
    // Perform init
    ADC_Status_T status = ADC_Init(&mLog, ADC_MAX_NUM_CHANNELS, 1);
    TEST_ASSERT(ADC_STATUS_OK == status);
    const char* expectedLogging = 
        "ADC_Init begin\n"
        "ADC_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // HAL_ADC_ConvHalfCpltCallback should do nothing
    ADC_HandleTypeDef hadc;
    HAL_ADC_ConvHalfCpltCallback(&hadc);

    for (uint16_t i = 0; i < ADC_MAX_NUM_CHANNELS; ++i) {
        uint16_t adcVal = ADC_Get(i);
        TEST_ASSERT(0 == adcVal);
    }
}

TEST(IO_ADC, TestAdcDataSingle)
{
    uint16_t testNumChannels = 4;
    // Perform init & config
    ADC_Status_T status = ADC_Init(&mLog, testNumChannels, 1);
    TEST_ASSERT(ADC_STATUS_OK == status);

    // Perform config
    ADC_HandleTypeDef hadc;
    status = ADC_Config(&hadc); // tie internal pointers for data
    TEST_ASSERT(ADC_STATUS_OK == status);
    const char* expectedLogging = 
        "ADC_Init begin\n"
        "ADC_Init complete\n"
        "ADC_Config begin\n"
        "ADC_Config complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // set data
    uint32_t dataRaw[4] = {0, 4095, 100, 0x1FFF};
    uint16_t dataExpected[4] = {0, 4095, 100, 0xFFF};
    mockSetADCData(dataRaw, sizeof(dataRaw));

    // Raise interrupt
    HAL_ADC_ConvCpltCallback(&hadc);

    for (uint16_t i = 0; i < numChannels; ++i) {
        uint16_t adcVal = ADC_Get(i);
        TEST_ASSERT(dataExpected[i] == adcVal);
    }
}

TEST(IO_ADC, TestAdcDataAvg)
{
    uint16_t testNumChannels = 6;
    // Perform init & config
    ADC_Status_T status = ADC_Init(&mLog, testNumChannels, 2);
    TEST_ASSERT(ADC_STATUS_OK == status);

    // Perform config
    ADC_HandleTypeDef hadc;
    status = ADC_Config(&hadc); // tie internal pointers for data
    TEST_ASSERT(ADC_STATUS_OK == status);
    const char* expectedLogging = 
        "ADC_Init begin\n"
        "ADC_Init complete\n"
        "ADC_Config begin\n"
        "ADC_Config complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // set data
    uint32_t dataRaw1[6]     = {0, 4095, 100, 4000, 4000, 0x1FFF};
    uint32_t dataRaw2[6]     = {0, 4000, 150, 4001, 4002, 0x1FFF};
    uint16_t dataExpected[6] = {0, 4047, 125, 4000, 4001, 4095};

    // Send first set of data
    mockSetADCData(dataRaw1, sizeof(dataRaw1));
    HAL_ADC_ConvCpltCallback(&hadc);

    // Send second set of data
    mockSetADCData(dataRaw2, sizeof(dataRaw2));
    HAL_ADC_ConvCpltCallback(&hadc);

    for (uint16_t i = 0; i < numChannels; ++i) {
        uint16_t adcVal = ADC_Get(i);
        TEST_ASSERT(dataExpected[i] == adcVal);
    }
}

TEST_GROUP_RUNNER(IO_ADC)
{
    RUN_TEST_CASE(IO_ADC, TestAdcInitOk);
    RUN_TEST_CASE(IO_ADC, TestAdcInitTooManyChannels);
    RUN_TEST_CASE(IO_ADC, TestAdcConfigOk);
    RUN_TEST_CASE(IO_ADC, TestAdcConfigDmaError);
    RUN_TEST_CASE(IO_ADC, TestAdcGetZero);
    RUN_TEST_CASE(IO_ADC, TestAdcInterruptHalf);
    RUN_TEST_CASE(IO_ADC, TestAdcDataSingle);
    RUN_TEST_CASE(IO_ADC, TestAdcDataAvg);
}

#define INVOKE_TEST IO_ADC
#include "test_main.h"
