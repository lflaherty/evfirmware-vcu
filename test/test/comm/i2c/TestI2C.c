/*
 * TestI2C.c
 *
 *  Created on: 7 Jul 2023
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"
#include <string.h>
#include <stdio.h>

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"

// source code under test
#include "comm/i2c/i2c.c"
 
static Logging_T testLog;

static I2C_HandleTypeDef hi2c2;

static const I2C_BusConfig_T configI2C = {
    .dev = I2C_DEV0,
    .handle = &hi2c2,
    .txIrq = DMA1_Stream4_IRQn,
    .rxIrq = DMA1_Stream2_IRQn,
};

TEST_GROUP(COMM_I2C);

TEST_SETUP(COMM_I2C)
{
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    mockSet_HAL_I2C_AllStatus(HAL_OK);

    // Enable interrupts (important for test as I2C code enables and disables these)
    HAL_NVIC_EnableIRQ(configI2C.txIrq);
    HAL_NVIC_EnableIRQ(configI2C.rxIrq);

    // Test setup
    TEST_ASSERT_EQUAL(I2C_STATUS_OK, I2C_Init(&testLog));
    TEST_ASSERT_EQUAL(I2C_STATUS_OK, I2C_Config(&configI2C));

    const char* expectedLogging = 
        "I2C_Init begin\n"
        "I2C_Init complete\n"
        "I2C_Config begin I2C0\n"
        "I2C_Config complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for the following test
    mockLogClear();
}

TEST_TEAR_DOWN(COMM_I2C)
{
    // Device shouldn't be left "busy"
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(i2cInstances[I2C_DEV0].busyMutex));

    mockLogClear();
    mockSet_HAL_I2C_AllStatus(HAL_OK);
}

TEST(COMM_I2C, TestI2CInitOk)
{
    // Done by TEST_SETUP
}

TEST(COMM_I2C, TestI2CWrite)
{
    uint16_t testAddr = 0x4A;
    uint8_t testData[3] = {0x11, 0x22, 0x33};
    uint16_t testDataLen = sizeof(testData) / sizeof(uint8_t);

    // Normally this would happen in parallel, half way through the function call,
    // but we can't really facilitate this in a single threaded test...
    // So just call the ISR to put the semaphore in the correct state
    HAL_I2C_MasterTxCpltCallback(&hi2c2);

    I2C_Status_T status;
    status = I2C_Write(I2C_DEV0, testAddr, testData, testDataLen);
    TEST_ASSERT_EQUAL(I2C_STATUS_OK, status);

    // Synchronization should be complete
    TEST_ASSERT_TRUE(mockSempahoreGetLocked(i2cInstances[I2C_DEV0].xferInProgressSem));
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(i2cInstances[I2C_DEV0].busyMutex));
    TEST_ASSERT_EQUAL(testAddr, mockGet_HAL_I2C_DevAddr());
    TEST_ASSERT_EQUAL(testDataLen, mockGet_HAL_I2C_DataBufLen());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(testData, mockGet_HAL_I2C_DataBuf(), testDataLen);

    // Now testing errors:

    // Do it again, with a timeout scenario
    // So don't trigger the sempahore
    mockSemaphoreSetCount(i2cInstances[I2C_DEV0].xferInProgressSem, 0);
    status = I2C_Write(I2C_DEV0, testAddr, testData, testDataLen);
    TEST_ASSERT_EQUAL(I2C_STATUS_ERROR_TIMEOUT, status);

    // now with an I2C device error
    mockSet_HAL_I2C_Master_Transmit_DMA_Status(HAL_ERROR);
    status = I2C_Write(I2C_DEV0, testAddr, testData, testDataLen);
    TEST_ASSERT_EQUAL(I2C_STATUS_ERROR_HAL, status);
    mockSet_HAL_I2C_Master_Transmit_DMA_Status(HAL_OK); // reset return value

    // now with too much data for the DMA buffer
    status = I2C_Write(I2C_DEV0, testAddr, testData, 10000);
    TEST_ASSERT_EQUAL(I2C_STATUS_ERROR_DATALEN, status);

    // and now with the device busy (another process is using it, and this has timed out)
    mockSemaphoreSetLocked(i2cInstances[I2C_DEV0].busyMutex, true);
    status = I2C_Write(I2C_DEV0, testAddr, testData, testDataLen);
    TEST_ASSERT_EQUAL(I2C_STATUS_ERROR_BUSY, status);

    // cleanup...
    mockSemaphoreSetCount(i2cInstances[I2C_DEV0].xferInProgressSem, 0);
    mockSemaphoreSetCount(i2cInstances[I2C_DEV0].busyMutex, 1);
}

TEST(COMM_I2C, TestI2CRead)
{
    uint16_t testAddr = 0x4B;
#define TEST_DATA_LEN 8
    uint8_t rxData[TEST_DATA_LEN];
    uint8_t testData[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
    _Static_assert(sizeof(testData) == TEST_DATA_LEN, "incorrect array size");

    // Apply test data to I2C mock
    mockSet_HAL_I2C_DataBuf(testData, TEST_DATA_LEN);

    // Normally this would happen in parallel, half way through the function call,
    // but we can't really facilitate this in a single threaded test...
    // So just call the ISR to put the semaphore in the correct state
    HAL_I2C_MasterRxCpltCallback(&hi2c2);

    I2C_Status_T status;
    status = I2C_Read(I2C_DEV0, testAddr, rxData, TEST_DATA_LEN);
    TEST_ASSERT_EQUAL(I2C_STATUS_OK, status);

    // Synchronization should be complete
    TEST_ASSERT_TRUE(mockSempahoreGetLocked(i2cInstances[I2C_DEV0].xferInProgressSem));
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(i2cInstances[I2C_DEV0].busyMutex));
    TEST_ASSERT_EQUAL(testAddr, mockGet_HAL_I2C_DevAddr());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(testData, rxData, TEST_DATA_LEN);

    // Now testing errors:

    // Do it again, with a timeout scenario
    // So don't trigger the sempahore
    mockSemaphoreSetCount(i2cInstances[I2C_DEV0].xferInProgressSem, 0);
    status = I2C_Read(I2C_DEV0, testAddr, rxData, TEST_DATA_LEN);;
    TEST_ASSERT_EQUAL(I2C_STATUS_ERROR_TIMEOUT, status);

    // now with an I2C device error
    mockSet_HAL_I2C_Master_Transmit_DMA_Status(HAL_ERROR);
    status = I2C_Read(I2C_DEV0, testAddr, rxData, TEST_DATA_LEN);
    TEST_ASSERT_EQUAL(I2C_STATUS_ERROR_HAL, status);
    mockSet_HAL_I2C_Master_Transmit_DMA_Status(HAL_OK); // reset return value

    // now with too much data for the DMA buffer
    status = I2C_Read(I2C_DEV0, testAddr, rxData, 10000);
    TEST_ASSERT_EQUAL(I2C_STATUS_ERROR_DATALEN, status);

    // and now with the device busy (another process is using it, and this has timed out)
    mockSemaphoreSetLocked(i2cInstances[I2C_DEV0].busyMutex, true);
    status = I2C_Read(I2C_DEV0, testAddr, rxData, TEST_DATA_LEN);
    TEST_ASSERT_EQUAL(I2C_STATUS_ERROR_BUSY, status);

    // cleanup...
    mockSemaphoreSetCount(i2cInstances[I2C_DEV0].xferInProgressSem, 0);
    mockSemaphoreSetCount(i2cInstances[I2C_DEV0].busyMutex, 1);
}

TEST_GROUP_RUNNER(COMM_I2C)
{
    RUN_TEST_CASE(COMM_I2C, TestI2CInitOk);
    RUN_TEST_CASE(COMM_I2C, TestI2CWrite);
    RUN_TEST_CASE(COMM_I2C, TestI2CRead);
    // TODO:
    // RUN_TEST_CASE(COMM_I2C, TestI2CMemWrite);
    // RUN_TEST_CASE(COMM_I2C, TestI2CMemRead);
}

#define INVOKE_TEST COMM_I2C
#include "test_main.h"
