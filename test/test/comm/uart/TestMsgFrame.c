/*
 * TestMsgFrame.c
 * 
 *  Created on: 25 Apr 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

// Mocks for code under test
#include "stm32_hal/MockStm32f7xx_hal.h"

// source code under test
#include "comm/uart/msgframe.c"

const uint16_t MSG_LEN = 11U;
MsgFrame_T mMsgFrame;

TEST_GROUP(COMM_MSGFRAME);

TEST_SETUP(COMM_MSGFRAME)
{
    memset(mMsgFrame.data, 0U, MSGFRAME_BUFFER_LEN * sizeof(uint8_t));
    mMsgFrame.msgLen = MSG_LEN;
    bool succ = MsgFrame_Init(&mMsgFrame);

    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(MSG_LEN, mMsgFrame.msgLen);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.start);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.end);
    TEST_ASSERT_EQUAL(MSGFRAME_BUFFER_LEN, mMsgFrame.availableBytes);
}

TEST_TEAR_DOWN(COMM_MSGFRAME)
{
    // Empty
}

TEST(COMM_MSGFRAME, TestMsgRecvBytes)
{
    bool succ;

    // Receive first message
    uint8_t msg1[] = {':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n'};
    size_t msg1Len = sizeof(msg1) / sizeof(uint8_t);

    succ = MsgFrame_RecvBytes(&mMsgFrame, msg1, (uint16_t)msg1Len);

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

    succ = MsgFrame_RecvBytes(&mMsgFrame, msg2, (uint16_t)msg2Len);

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

    succ = MsgFrame_RecvBytes(&mMsgFrame, msg3, (uint16_t)msg3Len);

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

TEST(COMM_MSGFRAME, TestMsgRecvMsg)
{
    // DMA might receive one message and some other bytes
    uint8_t msg1[] = {':', 0x01, 0x02, 0x03, 0x04, 0x4F, 0xCD, 0x55, 0x6F, '\r', '\n'};
    size_t msg1Len = sizeof(msg1) / sizeof(uint8_t);
    uint8_t msg2[] = {0x55, ':', '\r'};
    size_t msg2Len = sizeof(msg2) / sizeof(uint8_t);

    TEST_ASSERT_TRUE(MsgFrame_RecvBytes(&mMsgFrame, msg1, (uint16_t)msg1Len));
    TEST_ASSERT_TRUE(MsgFrame_RecvBytes(&mMsgFrame, msg2, (uint16_t)msg2Len));

    size_t offset;
    bool recv;

    // first attempt should work
    recv = MsgFrame_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_TRUE(recv);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg1, &mMsgFrame.data[offset], MSG_LEN);

    // second shouldn't give a valid message, but the bytes should be shifted down
    recv = MsgFrame_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_FALSE(recv);

    TEST_ASSERT_EQUAL(0U, mMsgFrame.start);
    TEST_ASSERT_EQUAL(msg2Len, mMsgFrame.end);
    TEST_ASSERT_EQUAL(MSGFRAME_BUFFER_LEN - (uint16_t)msg2Len, mMsgFrame.availableBytes);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg2, mMsgFrame.data, msg2Len);


    // now put another valid message after the random bytes
    TEST_ASSERT_TRUE(MsgFrame_RecvBytes(&mMsgFrame, msg1, (uint16_t)msg1Len));

    recv = MsgFrame_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_TRUE(recv);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg1, &mMsgFrame.data[offset], MSG_LEN);

    // buffer should be empty after another call
    recv = MsgFrame_RecvMsg(&mMsgFrame, &offset);
    TEST_ASSERT_FALSE(recv);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.start);
    TEST_ASSERT_EQUAL(0U, mMsgFrame.end);
    TEST_ASSERT_EQUAL(MSGFRAME_BUFFER_LEN, mMsgFrame.availableBytes);
}

TEST_GROUP_RUNNER(COMM_MSGFRAME)
{
    RUN_TEST_CASE(COMM_MSGFRAME, TestMsgRecvBytes);
    RUN_TEST_CASE(COMM_MSGFRAME, TestMsgRecvMsg);
    // TODO test incorrect CRC
}
