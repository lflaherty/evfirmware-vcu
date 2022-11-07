/*
 * TestNmeaFields.c
 * 
 *  Created on: 30 Oct 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>

// Mocks for code under test
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "lib/logging/MockLogging.h"

// source code under test
#include "comm/uart/nmeatypes.c"

TEST_GROUP(COMM_NMEATYPES);

TEST_SETUP(COMM_NMEATYPES)
{
    // No init to do
}

TEST_TEAR_DOWN(COMM_NMEATYPES)
{
    // Empty
}

TEST(COMM_NMEATYPES, TestUTCTime)
{
    NmeaUTCTime_T utcTime;
    char str[11] = "123456.789";

    TEST_ASSERT_TRUE(NmeaCharToUTCTime(str, &utcTime));
    TEST_ASSERT_EQUAL(12, utcTime.hour);
    TEST_ASSERT_EQUAL(34, utcTime.min);
    TEST_ASSERT_EQUAL(56, utcTime.sec);
    TEST_ASSERT_EQUAL(789, utcTime.millisec);

    char str2[11] = "1234t6.789";
    TEST_ASSERT_FALSE(NmeaCharToUTCTime(str2, &utcTime));
}

TEST(COMM_NMEATYPES, TestCharToUInt8Dec)
{
    uint8_t out = 0xFF;

    TEST_ASSERT_TRUE(charToUInt8Dec('0', &out));
    TEST_ASSERT_EQUAL(0, out);

    TEST_ASSERT_TRUE(charToUInt8Dec('5', &out));
    TEST_ASSERT_EQUAL(5, out);

    TEST_ASSERT_FALSE(charToUInt8Dec('f', &out));
    TEST_ASSERT_FALSE(charToUInt8Dec('F', &out));
    TEST_ASSERT_FALSE(charToUInt8Dec('-', &out));
    TEST_ASSERT_FALSE(charToUInt8Dec('u', &out));
}

TEST(COMM_NMEATYPES, TestCharToUInt16Dec)
{
    uint16_t out = 0xFF;

    TEST_ASSERT_TRUE(charToUInt16Dec('0', &out));
    TEST_ASSERT_EQUAL(0, out);

    TEST_ASSERT_TRUE(charToUInt16Dec('5', &out));
    TEST_ASSERT_EQUAL(5, out);

    TEST_ASSERT_FALSE(charToUInt16Dec('f', &out));
    TEST_ASSERT_FALSE(charToUInt16Dec('F', &out));
    TEST_ASSERT_FALSE(charToUInt16Dec('-', &out));
    TEST_ASSERT_FALSE(charToUInt16Dec('u', &out));
}

TEST(COMM_NMEATYPES, TestCharToUInt16Hex)
{
    uint16_t out = 0xFF;

    TEST_ASSERT_TRUE(charToUInt16Hex('0', &out));
    TEST_ASSERT_EQUAL(0, out);

    TEST_ASSERT_TRUE(charToUInt16Hex('5', &out));
    TEST_ASSERT_EQUAL(5, out);

    TEST_ASSERT_TRUE(charToUInt16Hex('f', &out));
    TEST_ASSERT_EQUAL(0xF, out);

    TEST_ASSERT_TRUE(charToUInt16Hex('F', &out));
    TEST_ASSERT_EQUAL(0xF, out);

    TEST_ASSERT_FALSE(charToUInt16Hex('-', &out));
    TEST_ASSERT_FALSE(charToUInt16Hex('u', &out));
}

TEST(COMM_NMEATYPES, TestStrToUInt8Dec)
{
    uint8_t out = 0xFF;

    char str1[] = "12";
    uint16_t str1Len = sizeof(str1) / sizeof(char) - 1;
    TEST_ASSERT_TRUE(strToUInt8Dec(str1, str1Len, &out));
}

TEST_GROUP_RUNNER(COMM_NMEATYPES)
{
    RUN_TEST_CASE(COMM_NMEATYPES, TestUTCTime);
    RUN_TEST_CASE(COMM_NMEATYPES, TestCharToUInt8Dec);
    RUN_TEST_CASE(COMM_NMEATYPES, TestCharToUInt16Dec);
    RUN_TEST_CASE(COMM_NMEATYPES, TestCharToUInt16Hex);
    RUN_TEST_CASE(COMM_NMEATYPES, TestStrToUInt8Dec);
}

#define INVOKE_TEST COMM_NMEATYPES
#include "test_main.h"
