/*
 * TestMsgFrameDecode.c
 * 
 *  Created on: 25 Apr 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

// Mocks for code under test
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "logging/MockLogging.h"

// source code under test
#include "uart/msgframedecode.c"

static const uint16_t MSG_LEN = 11U;

static Logging_T testLog;
static CRC_T mCrc;
static CRC_HandleTypeDef hcrc;

static MsgFrameDecode_T mMsgFrame;

TEST_GROUP(COMM_MSGFRAMEDECODE);

TEST_SETUP(COMM_MSGFRAMEDECODE)
{
    // set up supporting modules
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    mCrc.hcrc = &hcrc;
    TEST_ASSERT_EQUAL(CRC_STATUS_OK, CRC_Init(&testLog, &mCrc));

    // set up testing code
    memset(mMsgFrame.data, 0U, MSGFRAME_BUFFER_LEN * sizeof(uint8_t));
    mMsgFrame.msgLen = MSG_LEN;
    mMsgFrame.crc = &mCrc;
    bool succ = MsgFrameDecode_Init(&mMsgFrame);

    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(MSG_LEN, mMsgFrame.msgLen);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.start);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.end);
    TEST_ASSERT_EQUAL(MSGFRAME_BUFFER_LEN, mMsgFrame.availableBytes);
}

TEST_TEAR_DOWN(COMM_MSGFRAMEDECODE)
{
    // Empty
}

TEST(COMM_MSGFRAMEDECODE, TestInitTooLong)
{
    MsgFrameDecode_T mf2;
    mf2.msgLen = 1045U;
    mf2.crc = &mCrc;

    TEST_ASSERT_FALSE(MsgFrameDecode_Init(&mf2));
}

TEST(COMM_MSGFRAMEDECODE, TestMsgRecvBytes)
{
    mockSet_CRC(0x4FCD556FU);
    bool succ;

    // Receive first message
    uint8_t msg1[] = {':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n'};
    size_t msg1Len = sizeof(msg1) / sizeof(uint8_t);

    succ = MsgFrameDecode_RecvBytes(&mMsgFrame, msg1, (uint16_t)msg1Len);

    uint16_t expectedStart = 0U;
    uint16_t expectedEnd = (uint16_t)msg1Len;
    uint16_t expectedBufferRemaining = MSGFRAME_BUFFER_LEN - (uint16_t)msg1Len;
    uint8_t expectedBuffer1[] = {':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n'};
    size_t expectedBuffer1Len = sizeof(expectedBuffer1) / sizeof(uint8_t);
    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(MSG_LEN, mMsgFrame.msgLen);
    TEST_ASSERT_EQUAL(expectedStart, mMsgFrame.start);
    TEST_ASSERT_EQUAL(expectedEnd, mMsgFrame.end);
    TEST_ASSERT_EQUAL(expectedBufferRemaining, mMsgFrame.availableBytes);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedBuffer1, mMsgFrame.data, expectedBuffer1Len);

    // Receive some new bad data
    uint8_t msg2[] = {0x55, ':', '\r'};
    size_t msg2Len = sizeof(msg2) / sizeof(uint8_t);

    succ = MsgFrameDecode_RecvBytes(&mMsgFrame, msg2, (uint16_t)msg2Len);

    expectedStart += 0U;
    expectedEnd += msg2Len;
    expectedBufferRemaining -= msg2Len;
    uint8_t expectedBuffer2[] = {':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n',
                                 0x55, ':', '\r'};
    size_t expectedBuffer2Len = sizeof(expectedBuffer2) / sizeof(uint8_t);
    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(MSG_LEN, mMsgFrame.msgLen);
    TEST_ASSERT_EQUAL(expectedStart, mMsgFrame.start);
    TEST_ASSERT_EQUAL(expectedEnd, mMsgFrame.end);
    TEST_ASSERT_EQUAL(expectedBufferRemaining, mMsgFrame.availableBytes);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedBuffer2, mMsgFrame.data, expectedBuffer2Len);

    // Receive another valid message
    uint8_t msg3[] = {':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n'};
    size_t msg3Len = sizeof(msg3) / sizeof(uint8_t);

    succ = MsgFrameDecode_RecvBytes(&mMsgFrame, msg3, (uint16_t)msg3Len);

    expectedStart += 0U;
    expectedEnd += msg3Len;
    expectedBufferRemaining -= msg3Len;
    uint8_t expectedBuffer3[] = {':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n',
                                 0x55, ':', '\r',
                                 ':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n'};
    size_t expectedBuffer3Len = sizeof(expectedBuffer3) / sizeof(uint8_t);
    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(MSG_LEN, mMsgFrame.msgLen);
    TEST_ASSERT_EQUAL(expectedStart, mMsgFrame.start);
    TEST_ASSERT_EQUAL(expectedEnd, mMsgFrame.end);
    TEST_ASSERT_EQUAL(expectedBufferRemaining, mMsgFrame.availableBytes);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedBuffer3, mMsgFrame.data, expectedBuffer3Len);
}

TEST(COMM_MSGFRAMEDECODE, TestMsgRecvMsg)
{
    mockSet_CRC(0x4FCD556FU);

    // DMA might receive one message and some other bytes
    uint8_t msg1[] = {':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n'};
    size_t msg1Len = sizeof(msg1) / sizeof(uint8_t);
    uint8_t msg2[] = {0x55, ':', '\r'};
    size_t msg2Len = sizeof(msg2) / sizeof(uint8_t);

    TEST_ASSERT_TRUE(MsgFrameDecode_RecvBytes(&mMsgFrame, msg1, (uint16_t)msg1Len));
    TEST_ASSERT_TRUE(MsgFrameDecode_RecvBytes(&mMsgFrame, msg2, (uint16_t)msg2Len));

    size_t offset;
    bool recv;

    // first attempt should work
    recv = MsgFrameDecode_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_TRUE(recv);
    TEST_ASSERT_EQUAL(0U, offset);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg1, &mMsgFrame.data[offset], MSG_LEN);

    // second shouldn't give a valid message, but the bytes should be shifted down
    recv = MsgFrameDecode_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_FALSE(recv);

    TEST_ASSERT_EQUAL(0U, mMsgFrame.start);
    TEST_ASSERT_EQUAL(msg2Len, mMsgFrame.end);
    TEST_ASSERT_EQUAL(MSGFRAME_BUFFER_LEN - (uint16_t)msg2Len, mMsgFrame.availableBytes);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg2, mMsgFrame.data, msg2Len);


    // now put another valid message after the random bytes
    TEST_ASSERT_TRUE(MsgFrameDecode_RecvBytes(&mMsgFrame, msg1, (uint16_t)msg1Len));

    recv = MsgFrameDecode_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_TRUE(recv);
    TEST_ASSERT_EQUAL(3U, offset);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg1, &mMsgFrame.data[offset], MSG_LEN);

    // buffer should be empty after another call
    recv = MsgFrameDecode_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_FALSE(recv);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.start);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.end);
    TEST_ASSERT_EQUAL(MSGFRAME_BUFFER_LEN, mMsgFrame.availableBytes);
}

TEST(COMM_MSGFRAMEDECODE, TestMsgBadCrc)
{
    mockSet_CRC(0x4FCD556FU);

    uint8_t msgBadCrc[] = {':', 0x01, 0x02, 0x03, 0x04, 0xFF, 0xFF, 0xFF, 0xFF, '\r', '\n'};
    size_t msgBadCrcLen = sizeof(msgBadCrc) / sizeof(uint8_t);
    uint8_t msgGoodCrc[] = {':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n'};
    size_t msgGoodCrcLen = sizeof(msgGoodCrc) / sizeof(uint8_t);

    TEST_ASSERT_TRUE(MsgFrameDecode_RecvBytes(&mMsgFrame, msgBadCrc, (uint16_t)msgBadCrcLen));

    size_t offset;
    bool recv;

    // message with bad CRC should ignore message
    recv = MsgFrameDecode_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_FALSE(recv);

    TEST_ASSERT_TRUE(MsgFrameDecode_RecvBytes(&mMsgFrame, msgGoodCrc, (uint16_t)msgGoodCrcLen));

    // message with ok CRC should work
    recv = MsgFrameDecode_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_TRUE(recv);
    TEST_ASSERT_EQUAL(11U, offset);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msgGoodCrc, &mMsgFrame.data[offset], MSG_LEN);

    // check cleanup works ok
    recv = MsgFrameDecode_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_FALSE(recv);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.start);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.end);
    TEST_ASSERT_EQUAL(MSGFRAME_BUFFER_LEN, mMsgFrame.availableBytes);
}

TEST(COMM_MSGFRAMEDECODE, TestRecvTooManyBytes)
{
    uint8_t testByte1 = 0xAB;
    uint8_t testByte2 = 0xEF;

    // Fill up buffer ok
    for (uint16_t i = 0; i < MSGFRAME_BUFFER_LEN; ++i) {
        TEST_ASSERT_TRUE(MsgFrameDecode_RecvBytes(&mMsgFrame, &testByte1, sizeof(uint8_t)));
    }

    // Buffer should all be the correct bytes
    for (uint16_t i = 0; i < MSGFRAME_BUFFER_LEN; ++i) {
        TEST_ASSERT_EQUAL(testByte1, mMsgFrame.data[i]);
    }

    // Shouldn't be able to receive any more bytes
    for (uint16_t i = 0; i < MSGFRAME_BUFFER_LEN; ++i) {
        TEST_ASSERT_FALSE(MsgFrameDecode_RecvBytes(&mMsgFrame, &testByte2, sizeof(uint8_t)));
    }

    // Buffer shouldn't have changed
    for (uint16_t i = 0; i < MSGFRAME_BUFFER_LEN; ++i) {
        TEST_ASSERT_EQUAL(testByte1, mMsgFrame.data[i]);
    }
}

TEST_GROUP_RUNNER(COMM_MSGFRAMEDECODE)
{
    RUN_TEST_CASE(COMM_MSGFRAMEDECODE, TestInitTooLong);
    RUN_TEST_CASE(COMM_MSGFRAMEDECODE, TestMsgRecvBytes);
    RUN_TEST_CASE(COMM_MSGFRAMEDECODE, TestMsgRecvMsg);
    RUN_TEST_CASE(COMM_MSGFRAMEDECODE, TestMsgBadCrc);
    RUN_TEST_CASE(COMM_MSGFRAMEDECODE, TestRecvTooManyBytes);
}

#define INVOKE_TEST COMM_MSGFRAMEDECODE
#include "test_main.h"
