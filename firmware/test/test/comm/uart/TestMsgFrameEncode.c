/*
 * TestMsgFrameEncode.c
 * 
 *  Created on: 31 Jul 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

// Mocks for code under test
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "logging/MockLogging.h"

// source code under test
#include "uart/msgframeencode.c"

static const uint16_t MSG_LEN = 15U;
static const uint16_t DATA_LEN = 4U;

static Logging_T testLog;
static CRC_T mCrc;
static CRC_HandleTypeDef hcrc;

TEST_GROUP(COMM_MSGFRAMEENCODE);

TEST_SETUP(COMM_MSGFRAMEENCODE)
{
    // set up supporting modules
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    mCrc.hcrc = &hcrc;
    TEST_ASSERT_EQUAL(CRC_STATUS_OK, CRC_Init(&testLog, &mCrc));
}

TEST_TEAR_DOWN(COMM_MSGFRAMEENCODE)
{
    // Empty
}

TEST(COMM_MSGFRAMEENCODE, TestInitFrame)
{
    // Construct msg object
    uint8_t msgBuffer[MSG_LEN] = { 0U };
    MsgFrameEncode_T msg;
    msg.address = 0x102U;
    msg.function = 0x304U;
    msg.msgLen = MSG_LEN;
    msg.dataLen = DATA_LEN;
    msg.buffer = msgBuffer;
    msg.crc = &mCrc;

    uint8_t* msgData = MsgFrameEncode_InitFrame(&msg);
    TEST_ASSERT_POINTERS_EQUAL(msgData, msgBuffer + 5U);

    uint8_t expectedMsgBuffer[MSG_LEN] =
        {':', 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\r', '\n'};
        //    ^--Func--^  ^--Addr--^  ^--------Data--------^  ^---------CRC--------^  ^msg end-^
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgBuffer, msgBuffer, MSG_LEN);
}

TEST(COMM_MSGFRAMEENCODE, TestUpdateCrc)
{
    // Construct msg object
    uint8_t msgBuffer[MSG_LEN] = 
        {':', 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '\r', '\n'};
        //    ^--Func--^  ^--Addr--^  ^--------Data--------^  ^---------CRC--------^  ^msg end-^
    MsgFrameEncode_T msg;
    msg.address = 0x102U;
    msg.function = 0x304U;
    msg.msgLen = MSG_LEN;
    msg.dataLen = DATA_LEN;
    msg.buffer = msgBuffer;
    msg.crc = &mCrc;

    mockSet_CRC(0x4019276DU);
    uint8_t expectedMsgBuffer[MSG_LEN] = 
        {':', 0x01, 0x02, 0x03, 0x04, 0x00, 0x00, 0x00, 0x00, 0x40, 0x19, 0x27, 0x6D, '\r', '\n'};
        //    ^--Func--^  ^--Addr--^  ^--------Data--------^  ^---------CRC--------^  ^msg end-^

    MsgFrameEncode_UpdateCRC(&msg);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgBuffer, msgBuffer, MSG_LEN);
}

TEST_GROUP_RUNNER(COMM_MSGFRAMEENCODE)
{
    RUN_TEST_CASE(COMM_MSGFRAMEENCODE, TestInitFrame);
    RUN_TEST_CASE(COMM_MSGFRAMEENCODE, TestUpdateCrc);
}

#define INVOKE_TEST COMM_MSGFRAMEENCODE
#include "test_main.h"
