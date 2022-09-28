/*
 * TestCan.c
 *
 *  Created on: 23 Aug 2021
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
#include "comm/can/can.c"
 
static Logging_T testLog;

static const size_t recvQueueLen = 128;
static QueueHandle_t recvQueue;
static StaticQueue_t recvQueueBuffer;
static uint8_t recvQueueStorageArea[recvQueueLen];

// ISR
extern void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef* hcan);

TEST_GROUP(COMM_CAN);

TEST_SETUP(COMM_CAN)
{
    recvQueue = xQueueCreateStatic(
        recvQueueLen,
        sizeof(CAN_DataFrame_T),
        recvQueueStorageArea,
        &recvQueueBuffer);

    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    mockClearQueueData();
    mockSet_HAL_CAN_AllStatus(HAL_OK);
    mockClear_HAL_CAN_TxMailboxes();
    mockClear_HAL_CAN_RxFifo();
    
    // Test setup
    CAN_Status_T status = CAN_Init(&testLog);
    TEST_ASSERT_EQUAL(CAN_STATUS_OK, status);

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
    mockClearQueueData();
    mockSet_HAL_CAN_AllStatus(HAL_OK);
}

TEST(COMM_CAN, TestCanInitOk)
{
    // Done by TEST_SETUP
}

TEST(COMM_CAN, TestCanConfigOk)
{
    CAN_HandleTypeDef handle = {0};
    CAN_Status_T status = CAN_Config(CAN_DEV1, &handle);

    TEST_ASSERT_EQUAL(CAN_STATUS_OK, status);

    const char* expectedLogging = 
        "CAN_Config begin CAN1\n"
        "CAN_Config complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(COMM_CAN, TestCanConfigErrorCfg)
{
    CAN_HandleTypeDef handle = {0};
    mockSet_HAL_CAN_ConfigFilter_Status(HAL_ERROR);
    CAN_Status_T status = CAN_Config(CAN_DEV1, &handle);

    TEST_ASSERT_EQUAL(CAN_STATUS_ERROR_CFG_FILTER, status);

    const char* expectedLogging = "CAN_Config begin CAN1\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(COMM_CAN, TestCanConfigErrorStart)
{
    CAN_HandleTypeDef handle = {0};
    mockSet_HAL_CAN_Start_Status(HAL_ERROR);
    CAN_Status_T status = CAN_Config(CAN_DEV1, &handle);

    TEST_ASSERT_EQUAL(CAN_STATUS_ERROR_START, status);

    const char* expectedLogging = "CAN_Config begin CAN1\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(COMM_CAN, TestCanConfigErrorActivateNotification)
{
    CAN_HandleTypeDef handle = {0};
    mockSet_HAL_CAN_ActivateNotification_Status(HAL_ERROR);
    CAN_Status_T status = CAN_Config(CAN_DEV1, &handle);

    TEST_ASSERT_EQUAL(CAN_STATUS_ERROR_START_NOTIFY, status);

    const char* expectedLogging = "CAN_Config begin CAN1\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST(COMM_CAN, TestCanRegisterQueueOk)
{
    CAN_Status_T status = CAN_RegisterQueue(CAN_DEV1, 0x100, 0xF00, recvQueue);
    TEST_ASSERT_EQUAL(CAN_STATUS_OK, status);
}

TEST(COMM_CAN, TestCanRegisterQueuesFull)
{
    CAN_Status_T status;

    // fill up the callback queue
    for (uint8_t i = 0; i < CAN_MAX_RECV_QUEUES; ++i) {
        status = CAN_RegisterQueue(CAN_DEV1, 0x100, 0xF00, recvQueue);
        TEST_ASSERT_EQUAL(CAN_STATUS_OK, status);
    }

    // try to put in one more
    status = CAN_RegisterQueue(CAN_DEV1, 0x100, 0xF00, recvQueue);
    TEST_ASSERT_EQUAL(CAN_STATUS_ERROR_MAX_QUEUES, status);
}

TEST(COMM_CAN, TestCanSendOk)
{
    // construct data and send
    uint32_t msgIdSend = 0x100;
    uint8_t dataSend[8] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xFF};
    CAN_Status_T status = CAN_SendMessage(CAN_DEV1, msgIdSend, dataSend, 8);
    TEST_ASSERT_EQUAL(CAN_STATUS_OK, status);

    CAN_TxHeaderTypeDef* txHeader = mockGet_HAL_CAN_TxHeader(0);
    uint8_t* dataRecv = mockGet_HAL_CAN_TxData(0);
    TEST_ASSERT_EQUAL(1, mockGet_HAL_CAN_NumTxMailboxesInUse());
    TEST_ASSERT_EQUAL(8, txHeader->DLC);
    TEST_ASSERT_EQUAL(msgIdSend, txHeader->StdId);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(dataSend, dataRecv, 8);
}

TEST(COMM_CAN, TestCanSendBuffered)
{
    // Send lots of data, should append to the queue

    CAN_HandleTypeDef hcan = {0};
    TEST_ASSERT_EQUAL(CAN_STATUS_OK, CAN_Config(CAN_DEV1, &hcan));

    // Need to send more than 3 because there are 3 mailboxes
    uint32_t msgIds[] = {
        0x101,
        0x102,
        0x103,
        0x104
    };
    uint8_t dataSend[][8] = {
        {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7},
        {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7},
        {0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7},
        {0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7}
    };
    size_t numTestMessages = sizeof(msgIds) / sizeof(msgIds[0]);
    TEST_ASSERT_EQUAL(numTestMessages, sizeof(dataSend) / 8);

    for (size_t i = 0; i < numTestMessages; ++i) {
        CAN_Status_T status = CAN_SendMessage(CAN_DEV1, msgIds[i], dataSend[i], 8);
        TEST_ASSERT_EQUAL(CAN_STATUS_OK, status);
    }
    
    // First three messages in tx mailboxes, and third waiting in queue
    TEST_ASSERT_EQUAL(3U, mockGet_HAL_CAN_NumTxMailboxesInUse());
    TEST_ASSERT_EQUAL(1U * TX_PENDING_ITEM_SIZE, mockGetQueueSize());
    for (uint32_t i = 0; i < 3; ++i) {
        CAN_TxHeaderTypeDef* txHeader = mockGet_HAL_CAN_TxHeader(i);
        uint8_t* dataRecv = mockGet_HAL_CAN_TxData(i);
        TEST_ASSERT_EQUAL(8, txHeader->DLC);
        TEST_ASSERT_EQUAL(msgIds[i], txHeader->StdId);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(dataSend[i], dataRecv, 8);
    }

    // Call tx complete callback, should transmit pending messages
    HAL_CAN_TxMailbox0CompleteCallback(&hcan);
    HAL_CAN_TxMailbox1CompleteCallback(&hcan);
    HAL_CAN_TxMailbox2CompleteCallback(&hcan);
    TEST_ASSERT_EQUAL(3U, mockGet_HAL_CAN_NumTxMailboxesInUse());
    for (uint32_t i = 0; i < 3U; ++i) {
        CAN_TxHeaderTypeDef* txHeader = mockGet_HAL_CAN_TxHeader(i);
        uint8_t* txData = mockGet_HAL_CAN_TxData(i);

        TEST_ASSERT_EQUAL(msgIds[i], txHeader->StdId);
        TEST_ASSERT_EQUAL(8, txHeader->DLC);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(dataSend[i], txData, 8);
    }

    // Clear the mock mailboxes and continue sending
    mockClear_HAL_CAN_TxMailboxes();
    HAL_CAN_TxMailbox0CompleteCallback(&hcan);
    TEST_ASSERT_EQUAL(1U, mockGet_HAL_CAN_NumTxMailboxesInUse());
    TEST_ASSERT_EQUAL(0U, mockGetQueueSize());

    CAN_TxHeaderTypeDef* txHeader = mockGet_HAL_CAN_TxHeader(0);
    uint8_t* txData = mockGet_HAL_CAN_TxData(0);
    TEST_ASSERT_EQUAL(msgIds[3], txHeader->StdId);
    TEST_ASSERT_EQUAL(8, txHeader->DLC);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(dataSend[3], txData, 8);
}

TEST(COMM_CAN, TestCanSendError)
{
    // construct data and send (but set an error)
    uint32_t msgIdSend = 0x100;
    uint8_t dataSend[8] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xFF};
    mockSet_HAL_CAN_AddTxMessage_Status(HAL_ERROR);

    CAN_Status_T status = CAN_SendMessage(CAN_DEV1, msgIdSend, dataSend, 8);
    TEST_ASSERT_EQUAL(CAN_STATUS_ERROR_TX, status);
}

TEST(COMM_CAN, TestCanReceive)
{
    CAN_HandleTypeDef hcan;
    hcan.Instance = CAN1;

    TEST_ASSERT_EQUAL(CAN_STATUS_OK, CAN_Config(CAN_DEV1, &hcan));

    uint32_t dviceMask = 0xF00;
    uint32_t device1Id = 0x100;

    // construct data to be received
    uint32_t dlc = 8;
    uint32_t msg1Id = 0x10C;
    uint32_t msg2Id = 0x420;
    uint8_t data1[8] = {0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0xFF};
    uint8_t data2[8] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x1F};

    // register the callback
    CAN_Status_T status = CAN_RegisterQueue(CAN_DEV1, device1Id, dviceMask, recvQueue);
    TEST_ASSERT_EQUAL(CAN_STATUS_OK, status);

    // invoke callback
    mockAddHALCANRxMessage(msg1Id, data1, dlc);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);

    // Should have 1 CAN_DataFrame_T object in the queue now
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize());

    CAN_DataFrame_T recvData;
    mockGetQueueData(&recvData, sizeof(CAN_DataFrame_T));
    TEST_ASSERT_EQUAL(CAN_DEV1, recvData.busInstance);
    TEST_ASSERT_EQUAL(msg1Id, recvData.msgId);
    TEST_ASSERT_EQUAL(dlc, recvData.dlc);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(data1, recvData.data, 8U);

    mockClearQueueData();

    // now try a message that should be filtered out
    mockAddHALCANRxMessage(msg2Id, data2, dlc);
    HAL_CAN_RxFifo1MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(0, mockGetQueueSize());
}

TEST_GROUP_RUNNER(COMM_CAN)
{
    RUN_TEST_CASE(COMM_CAN, TestCanInitOk);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigOk);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigErrorCfg);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigErrorStart);
    RUN_TEST_CASE(COMM_CAN, TestCanConfigErrorActivateNotification);
    RUN_TEST_CASE(COMM_CAN, TestCanRegisterQueueOk);
    RUN_TEST_CASE(COMM_CAN, TestCanRegisterQueuesFull);
    RUN_TEST_CASE(COMM_CAN, TestCanSendOk);
    RUN_TEST_CASE(COMM_CAN, TestCanSendBuffered);
    RUN_TEST_CASE(COMM_CAN, TestCanSendError);
    RUN_TEST_CASE(COMM_CAN, TestCanReceive);
}