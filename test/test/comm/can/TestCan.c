/*
 * TestCan.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"
#include <string.h>

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lib/logging/MockLogging.h"

// source code under test
#include "comm/can/can.c"
 
static Logging_T mLog;

/**
 * Storage for CAN data from callbacks for tests
 */
static uint32_t canMsgId1 = 0;
static uint32_t canMsgId2 = 0;
static uint8_t canData1[8] = {0};
static uint8_t canData2[8] = {0};

// ISR
extern void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan);

void CAN_Callback1(const CAN_DataFrame_T* data, const void* param)
{
    (void)param;

    memcpy(canData1, data->data, data->dlc);
    canMsgId1 = data->msgId;
}

void CAN_Callback2(const CAN_DataFrame_T* data, const void* param)
{
    (void)param;

    memcpy(canData2, data->data, data->dlc);
    canMsgId2 = data->msgId;
}

TEST_GROUP(COMM_CAN);

TEST_SETUP(COMM_CAN)
{
    mLog.enableLogToDebug = true;
    mLog.enableLogToLogFile = false;
    mLog.enableLogToSerial = false;
    mockLogClear();
    mockSet_HAL_CAN_AllStatus(HAL_OK);
    
    // Test setup
    CAN_Status_T status = CAN_Init(&mLog);
    TEST_ASSERT(CAN_STATUS_OK == status);

    const char* expectedLogging = 
        "CAN_Init begin\n"
        "CAN_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for the following test
    mockLogClear();
}

TEST_TEAR_DOWN(COMM_CAN)
{
    mockLogClear();
    mockSet_HAL_CAN_AllStatus(HAL_OK);
}

TEST(COMM_CAN, TestCanInitOk)
{
    // Done by TEST_SETUP
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

TEST(COMM_CAN, TestCanRegisterCallbackOk)
{
    CAN_HandleTypeDef handle;
    CAN_Status_T status = CAN_RegisterCallback(&handle, 0x100, CAN_Callback1, NULL);
    TEST_ASSERT(CAN_STATUS_OK == status);
}

TEST(COMM_CAN, TestCanRegisterCallbackFull)
{
    CAN_Status_T status;

    // fill up the callback queue
    CAN_HandleTypeDef handle;
    for (uint8_t i = 0; i < CAN_NUM_CALLBACKS; ++i) {
        status = CAN_RegisterCallback(&handle, 0x100, CAN_Callback1, NULL);
        TEST_ASSERT(CAN_STATUS_OK == status);
    }

    // try to put in one more
    status = CAN_RegisterCallback(&handle, 0x100, CAN_Callback1, NULL);
    TEST_ASSERT(CAN_STATUS_ERROR_CALLBACK_FULL == status);
}

TEST(COMM_CAN, TestCanSendOk)
{
    // construct data and send
    CAN_HandleTypeDef handle;
    uint32_t msgIdSend = 0x100;
    uint8_t dataSend[8] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xFF};
    CAN_Status_T status = CAN_SendMessage(&handle, msgIdSend, dataSend, 8);
    TEST_ASSERT(CAN_STATUS_OK == status);

    uint8_t dataRecv[8] = {0};
    CAN_RxHeaderTypeDef headerRecv = {0};
    HAL_CAN_GetRxMessage(&handle, 0, &headerRecv, dataRecv); // use this method to extract data from mock

    TEST_ASSERT(8 == headerRecv.DLC);
    TEST_ASSERT(msgIdSend == headerRecv.StdId);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(dataSend, dataRecv, 8);
}

TEST(COMM_CAN, TestCanSendError)
{
    // construct data and send (but set an error)
    CAN_HandleTypeDef handle;
    uint32_t msgIdSend = 0x100;
    uint8_t dataSend[8] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xFF};
    mockSet_HAL_CAN_AddTxMessage_Status(HAL_ERROR);

    CAN_Status_T status = CAN_SendMessage(&handle, msgIdSend, dataSend, 8);
    TEST_ASSERT(CAN_STATUS_ERROR_TX == status);
}

TEST(COMM_CAN, TestCanReceive)
{
    // construct data to be received
    CAN_HandleTypeDef handle;
    uint32_t msgId = 0x100;
    uint8_t data[8] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xFF};

    // register the callback
    CAN_Status_T status = CAN_RegisterCallback(&handle, 0x100, CAN_Callback1, NULL);
    TEST_ASSERT(CAN_STATUS_OK == status);

    // TODO continue test
    (void)msgId;
    (void)data;
}

TEST_GROUP_RUNNER(COMM_CAN)
{
    RUN_TEST_CASE(COMM_CAN, TestCanInitOk);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigOk);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigErrorCfg);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigErrorStart);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigErrorActivateNotification);
    RUN_TEST_CASE(COMM_CAN, TestCanRegisterCallbackOk);
    RUN_TEST_CASE(COMM_CAN, TestCanRegisterCallbackFull);
    RUN_TEST_CASE(COMM_CAN, TestCanSendOk);
    RUN_TEST_CASE(COMM_CAN, TestCanSendError);
}