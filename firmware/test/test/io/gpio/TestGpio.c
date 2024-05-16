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
#include "gpio/gpio.c"

static GPIO_TypeDef mGpioBank;
static uint16_t mGpioPin = 1;
static GPIO_T mGpio; // only used to pass fields to STM32 HAL

TEST_GROUP(IO_GPIO);

TEST_SETUP(IO_GPIO)
{
    mGpio.GPIOx = &mGpioBank;
    mGpio.GPIO_Pin = mGpioPin;

    mock_GPIO_RegisterPin(&mGpioBank, mGpioPin);
    mockSet_GPIO_Asserted(&mGpioBank, mGpioPin, false);
}

TEST_TEAR_DOWN(IO_GPIO)
{
    // Empty
}

TEST(IO_GPIO, TestGpioRead)
{
    mockSet_GPIO_Asserted(&mGpioBank, mGpioPin, false);
    TEST_ASSERT_FALSE(GPIO_ReadPin(&mGpio));

    mockSet_GPIO_Asserted(&mGpioBank, mGpioPin, true);
    TEST_ASSERT_TRUE(GPIO_ReadPin(&mGpio));

    mockSet_GPIO_Asserted(&mGpioBank, mGpioPin, false);
    TEST_ASSERT_FALSE(GPIO_ReadPin(&mGpio));
}

TEST(IO_GPIO, TestGpioWrite)
{
    for (int i = 0; i < 10; ++i) {
        GPIO_WritePin(&mGpio, true);
        TEST_ASSERT_TRUE(mockGet_GPIO_Asserted(&mGpioBank, mGpioPin));
        TEST_ASSERT_TRUE(GPIO_ReadPin(&mGpio));
    }
    
    for (int i = 0; i < 10; i++) {
        GPIO_WritePin(&mGpio, false);
        TEST_ASSERT_FALSE(mockGet_GPIO_Asserted(&mGpioBank, mGpioPin));
        TEST_ASSERT_FALSE(GPIO_ReadPin(&mGpio));
    }
}

TEST(IO_GPIO, TestGpioToggle)
{
    bool expected = false;
    for (int i = 0; i < 20; ++i) {
        expected = !expected;
        GPIO_TogglePin(&mGpio);
        TEST_ASSERT_EQUAL(expected, mockGet_GPIO_Asserted(&mGpioBank, mGpioPin));
        TEST_ASSERT_EQUAL(expected, GPIO_ReadPin(&mGpio));
    }
}

TEST_GROUP_RUNNER(IO_GPIO)
{
    RUN_TEST_CASE(IO_GPIO, TestGpioRead);
    RUN_TEST_CASE(IO_GPIO, TestGpioWrite);
    RUN_TEST_CASE(IO_GPIO, TestGpioToggle);
}

#define INVOKE_TEST IO_GPIO
#include "test_main.h"
