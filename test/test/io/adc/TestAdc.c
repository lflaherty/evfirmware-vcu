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

static Logging_T testLog;

static ADC_HandleTypeDef hadc1 = {
    .Instance = ADC1,
    .Init = (ADC_InitTypeDef){
        .ContinuousConvMode = ENABLE,
        .DataAlign = ADC_DATAALIGN_RIGHT,
        .NbrOfConversion = 0, // configured in each test
    }
};
static ADC_Config_T adcConfig = {
    .logger = &testLog,
    .handle = &hadc1,
    .adcIrq = DMA2_Stream0_IRQn,
    .numChannelsUsed = 0, // configured in each test
};

// HAL interrupts:
extern void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc);
extern void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc);

static void setTestNumChannels(uint16_t testNumChannels)
{
    hadc1.Init.NbrOfConversion = testNumChannels;
    adcConfig.numChannelsUsed = testNumChannels;
}

TEST_GROUP(IO_ADC);

TEST_SETUP(IO_ADC)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    mockSet_HAL_ADC_Start_DMA_Status(HAL_OK);

    // Reset number of channels for test
    setTestNumChannels(0);
}

TEST_TEAR_DOWN(IO_ADC)
{
    mockLogClear();
    mockSet_HAL_ADC_Start_DMA_Status(HAL_OK);
}

TEST(IO_ADC, TestAdcInitOk)
{
    ADC_Status_T status = ADC_Init(&adcConfig);

    TEST_ASSERT_EQUAL(ADC_STATUS_OK, status);

    const char* expectedLogging = 
        "ADC_Init begin\n"
        "ADC_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(IO_ADC, TestAdcInitTooManyChannels)
{
    // Reset number of channels for test
    setTestNumChannels(ADC_MAX_NUM_CHANNELS+1);
    ADC_Status_T status = ADC_Init(&adcConfig);

    TEST_ASSERT_EQUAL(ADC_STATUS_ERROR_CHANNEL_COUNT, status);

    const char* expectedLogging = 
        "ADC_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(IO_ADC, TestAdcConfigOk)
{
    // Perform init
    setTestNumChannels(ADC_MAX_NUM_CHANNELS);
    TEST_ASSERT_EQUAL(ADC_STATUS_OK, ADC_Init(&adcConfig));

    const char* expectedLogging = 
        "ADC_Init begin\n"
        "ADC_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(IO_ADC, TestAdcConfigDmaError)
{
    setTestNumChannels(ADC_MAX_NUM_CHANNELS);
    mockSet_HAL_ADC_Start_DMA_Status(HAL_ERROR);

    // Perform init
    ADC_Status_T status = ADC_Init(&adcConfig);
    TEST_ASSERT_EQUAL(ADC_STATUS_ERROR_DMA, status);

    const char* expectedLogging = 
        "ADC_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(IO_ADC, TestAdcGetZero)
{
    // Perform init
    setTestNumChannels(ADC_MAX_NUM_CHANNELS);
    TEST_ASSERT_EQUAL(ADC_STATUS_OK, ADC_Init(&adcConfig));

    // Attempt to get
    for (uint16_t i = 0; i < ADC_MAX_NUM_CHANNELS; ++i) {
        uint16_t adcVal = 0xFFFF;
        TEST_ASSERT_EQUAL(ADC_STATUS_OK, ADC_Get(i, &adcVal));
        TEST_ASSERT_EQUAL(0, adcVal);
    }

    // Get invalid value
    uint16_t adcVal = 0xFFFF;
    ADC_Status_T status = ADC_Get(ADC_MAX_NUM_CHANNELS+1, &adcVal);
    TEST_ASSERT_EQUAL(ADC_STATUS_ERROR_INVALID_CHANNEL, status);
    TEST_ASSERT_EQUAL(0, adcVal);
}

TEST(IO_ADC, TestAdcInterruptHalf)
{
    // Perform init
    setTestNumChannels(ADC_MAX_NUM_CHANNELS);
    TEST_ASSERT_EQUAL(ADC_STATUS_OK, ADC_Init(&adcConfig));

    // HAL_ADC_ConvHalfCpltCallback should do nothing to API
    HAL_ADC_ConvHalfCpltCallback(&hadc1);

    for (uint16_t i = 0; i < ADC_MAX_NUM_CHANNELS; ++i) {
        uint16_t adcVal = 0xFFFF;
        TEST_ASSERT_EQUAL(ADC_STATUS_OK, ADC_Get(i, &adcVal));
        TEST_ASSERT_EQUAL(0, adcVal);
    }
}

TEST(IO_ADC, TestAdcDataSingle)
{
    uint16_t testNumChannels = 5;
    // Perform init
    setTestNumChannels(testNumChannels);
    TEST_ASSERT_EQUAL(ADC_STATUS_OK, ADC_Init(&adcConfig));

    // set data
    uint32_t dataRaw[5] = {1, 4095, 100, 0, 0x1FFF};
    mockSetADCData(dataRaw, sizeof(dataRaw));

    // Raise interrupts
    // need to do the half cplt first as well to do the copying
    HAL_ADC_ConvHalfCpltCallback(&hadc1);
    HAL_ADC_ConvCpltCallback(&hadc1);

    for (uint16_t i = 0; i < numChannels; ++i) {
        uint16_t adcVal = 0xFFFF;
        TEST_ASSERT_EQUAL(ADC_STATUS_OK, ADC_Get(i, &adcVal));
        TEST_ASSERT_EQUAL(dataRaw[i], adcVal);
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
}

#define INVOKE_TEST IO_ADC
#include "test_main.h"
