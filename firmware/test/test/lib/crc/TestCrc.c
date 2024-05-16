/*
 * TestMsgFrameDecode.c
 * 
 *  Created on: 20 Oct 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

// Mocks for code under test
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"

#include "logging/MockLogging.h"

// source code under test
#include "crc/crc.c"

static Logging_T testLog;
static CRC_HandleTypeDef hcrc;

static CRC_T mCrc;

TEST_GROUP(LIB_CRC);

TEST_SETUP(LIB_CRC)
{
    // set up supporting modules
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;

    // start CRC
    CRC_Status_T status = CRC_Init(&testLog, &mCrc);

    TEST_ASSERT_EQUAL(CRC_STATUS_OK, status);
}

TEST_TEAR_DOWN(LIB_CRC)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(mCrc.mutex));
}

TEST(LIB_CRC, TestInitOk)
{
    // Tested in TEST_SETUP
}

TEST(LIB_CRC, TestCrcOK)
{
    const uint32_t testCrc = 0x12345678U;
    mockSet_CRC(testCrc);

    // This doesn't matter since the CRC hardware is mocked, but we need a value
    uint8_t tempBuffer[4] = {1, 2, 3, 4};
    uint32_t bufLen = 4;
    TickType_t timeout = 10U;

    uint32_t crc = 0xFFFFFFFF;
    bool ret = CRC_Calculate(
        &mCrc,
        (void*)tempBuffer,
        bufLen,
        timeout,
        &crc);
    
    TEST_ASSERT_TRUE(ret);
    TEST_ASSERT_EQUAL(testCrc, crc);
}

TEST(LIB_CRC, TestMutexTimeout)
{
    const uint32_t testCrc = 0x12345678U;
    mockSet_CRC(testCrc);
    mockSemaphoreSetLocked(mCrc.mutex, true);

    // This doesn't matter since the CRC hardware is mocked, but we need a value
    uint8_t tempBuffer[4] = {1, 2, 3, 4};
    uint32_t bufLen = 4;
    TickType_t timeout = 10U;

    uint32_t crc = 0xFFFFFFFF;
    bool ret = CRC_Calculate(
        &mCrc,
        (void*)tempBuffer,
        bufLen,
        timeout,
        &crc);
    
    TEST_ASSERT_FALSE(ret);
    TEST_ASSERT_EQUAL(0U, crc);

    mockSemaphoreSetLocked(mCrc.mutex, false);
}

TEST_GROUP_RUNNER(LIB_CRC)
{
    RUN_TEST_CASE(LIB_CRC, TestInitOk);
    RUN_TEST_CASE(LIB_CRC, TestCrcOK);
    RUN_TEST_CASE(LIB_CRC, TestMutexTimeout);
}

#define INVOKE_TEST LIB_CRC
#include "test_main.h"
