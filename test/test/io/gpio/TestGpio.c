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

// source code under test
#include "io/gpio/gpio.c"

GPIO_T mGpioPin; // only used to pass fields to STM32 HAL

TEST_GROUP(IO_GPIO);

TEST_SETUP(IO_GPIO)
{
    mockSet_GPIO_Asserted(false);
    mockSet_HAL_ADC_Start_DMA_Status(HAL_OK);
}

TEST_TEAR_DOWN(IO_GPIO)
{
    mockSet_HAL_ADC_Start_DMA_Status(HAL_OK);
}

TEST(IO_GPIO, TestGpioRead)
{
    mockSet_GPIO_Asserted(false);
    TEST_ASSERT_FALSE(GPIO_ReadPin(&mGpioPin));

    mockSet_GPIO_Asserted(true);
    TEST_ASSERT_TRUE(GPIO_ReadPin(&mGpioPin));

    mockSet_GPIO_Asserted(false);
    TEST_ASSERT_FALSE(GPIO_ReadPin(&mGpioPin));
}

TEST(IO_GPIO, TestGpioWrite)
{
    for (int i = 0; i < 10; ++i) {
        GPIO_WritePin(&mGpioPin, true);
        TEST_ASSERT_TRUE(mockGet_GPIO_Asserted());
        TEST_ASSERT_TRUE(GPIO_ReadPin(&mGpioPin));
    }
    
    for (int i = 0; i < 10; i++) {
        GPIO_WritePin(&mGpioPin, false);
        TEST_ASSERT_FALSE(mockGet_GPIO_Asserted());
        TEST_ASSERT_FALSE(GPIO_ReadPin(&mGpioPin));
    }
}

TEST(IO_GPIO, TestGpioToggle)
{
    bool expected = false;
    for (int i = 0; i < 20; ++i) {
        expected = !expected;
        GPIO_TogglePin(&mGpioPin);
        TEST_ASSERT_EQUAL(expected, mockGet_GPIO_Asserted());
        TEST_ASSERT_EQUAL(expected, GPIO_ReadPin(&mGpioPin));
    }
}

TEST_GROUP_RUNNER(IO_GPIO)
{
    RUN_TEST_CASE(IO_GPIO, TestGpioRead);
    RUN_TEST_CASE(IO_GPIO, TestGpioWrite);
    RUN_TEST_CASE(IO_GPIO, TestGpioToggle);
}