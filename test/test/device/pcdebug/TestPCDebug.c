/**
 * TestPCDebug.c
 * 
 *  Created on: Aug 2 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

// Mocks for code under test (replaces stubs)
#include "std/MockStdio.h"
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stream_buffer.h"
#include "semaphore.h"

#include "time/tasktimer/MockTasktimer.h"
// MockLogging.h is deliberately not used here - need the stream internals of
// logging to work correctly. Use MockStdio to capture SWO printfs instead

// Prod (non mock) methods to avoid linker errors
// TODO there's probably a better way to do this...
#define UART_Init implUART_Init
#define UART_SetRecvStream implUART_SetRecvStream
#define UART_Config implUART_Config
#define UART_SendMessage implUART_SendMessage
#define HAL_UARTEx_RxEventCallback implHAL_UARTEx_RxEventCallback
#define HAL_UART_TxCpltCallback implHAL_UART_TxCpltCallback
#define HAL_UART_ErrorCallback implHAL_UART_ErrorCallback
#define MsgFrameEncode_InitFrame implMsgFrameEncode_InitFrame
#define MsgFrameEncode_UpdateCRC implMsgFrameEncode_UpdateCRC

// source code under test
#include "comm/uart/uart.c"
#include "comm/uart/msgframeencode.c"
#include "device/pcdebug/pcdebug.c"

static Logging_T testLog;
static CRC_HandleTypeDef hcrc;
static UART_HandleTypeDef husart1;
static PCDebug_T mPCDebug;

// Helper macro for changing endianness
#define BIG_TO_LITTLE_ENDIAN_U32(x) (((x >> 24) & 0xff) | \
                                     ((x << 8) & 0xff0000) | \
                                     ((x >> 8) & 0xff00) | \
                                     ((x << 24) & 0xff000000))

TEST_GROUP(DEVICE_PCDEBUG);

TEST_SETUP(DEVICE_PCDEBUG)
{
    mockSemaphoreSetLocked(false);

    // Init logging
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));

    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);

    // Init CRC
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;

    // Init UART
    husart1.Instance = USART1;
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_Init(&testLog));
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_Config(&husart1));

    // clear to only capture PC debug prints
    mockClearPrintf();
    
    // Init PC Debug
    memset(&mPCDebug, 0U, sizeof(PCDebug_T));
    mPCDebug.huart = &husart1;
    mPCDebug.hcrc = &hcrc;

    PCDebug_Status_T status = PCDebug_Init(&testLog, &mPCDebug);
    TEST_ASSERT(PCDEBUG_STATUS_OK == status);

    const char* expectedLogging =
        "PCDebug_Init begin\n"
        "PCDebug_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, printfOut);

    // clear again for coming tests (because init prints)
    mockClearStreamBufferData(mPCDebug.logStreamHandle);
    mockClearStreamBufferData(usart1.txPendingStreamHandle);
    mockClear_HAL_UART_Data();
    mockClearPrintf();
}

TEST_TEAR_DOWN(DEVICE_PCDEBUG)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());
    mockClear_HAL_UART_Data();
}

TEST(DEVICE_PCDEBUG, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_PCDEBUG, InitTaskRegisterError)
{
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_ERROR_FULL);

    PCDebug_Status_T status = PCDebug_Init(&testLog, &mPCDebug);
    TEST_ASSERT(PCDEBUG_STATUS_ERROR_INIT == status);

    const char* expectedLogging =
        "PCDebug_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, printfOut);
}

TEST(DEVICE_PCDEBUG, TestNoMessages)
{
    const char simpleMsg[] = "Hello!\n"; // less than 32 data chars
    assert(sizeof(simpleMsg) <= 32U);

    // With no log messages, nothing should happen

    // First 500ms will attempt to print twice...
    for (uint16_t i = 0; i < 5U; ++i) {
        mockSetTaskNotifyValue(1); // to wake up
        PCDebug_TaskMethod(&mPCDebug);
        TEST_ASSERT_EQUAL(0U, mockGet_HAL_UART_Len());
    }
}

TEST(DEVICE_PCDEBUG, TestLogSerialShortMsg)
{
    const char simpleMsg[] = "Hello!\n"; // less than 32 data chars
    assert(sizeof(simpleMsg) <= 32U);

    mockSet_CRC(BIG_TO_LITTLE_ENDIAN_U32(0xAABBCCDD));
    const uint8_t expectedUart[PCDEBUG_MSG_LOG_MSGLEN] = {
        ':',  // Start byte
        0x00, 0x02,     // Receiver address: Debug PC
        0x00, 0x02,     // Message ID: log message
        'H', 'e', 'l', 'l', 'o', '!', '\n',
        0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, // Zero padding for unused bytes
        0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
        0U, 0U, 0U, 0U, 0U,
        0xAA, 0xBB, 0xCC, 0xDD, // CRC bytes (mock bytes here)
        '\r', '\n'      // End of message frame
    };

    Log_Print(&testLog, simpleMsg);

    // serial should be flushed
    mockSetTaskNotifyValue(1); // to wake up
    PCDebug_TaskMethod(&mPCDebug);

    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedUart, mPCDebug.mfLogDataBuffer, PCDEBUG_MSG_LOG_MSGLEN);
    TEST_ASSERT_EQUAL(PCDEBUG_MSG_LOG_MSGLEN, mockGet_HAL_UART_Len());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedUart, mockGet_HAL_UART_Data(), PCDEBUG_MSG_LOG_MSGLEN);
}

TEST(DEVICE_PCDEBUG, TestLogSerialLongMsg)
{
    // A log message that needs to be split into two message (i.e. longer than 32)
    const char longMsg[] = "Never gonna give you up\n"
                           "Never gonna let you down\n";
    assert(sizeof(longMsg) > 32U);

    mockSet_CRC(BIG_TO_LITTLE_ENDIAN_U32(0xAABBCCDD));
    uint8_t expectedUart1[PCDEBUG_MSG_LOG_MSGLEN] = { 0 };
    uint8_t expectedUart2[PCDEBUG_MSG_LOG_MSGLEN] = { 0 };

    expectedUart1[0] = ':';
    expectedUart1[2] = 0x02; // Receiver address: Debug PC
    expectedUart1[4] = 0x02; // Message ID: log message
    memcpy(expectedUart1 + 5, longMsg, 32);
    expectedUart1[37] = 0xAA; // CRC
    expectedUart1[38] = 0xBB;
    expectedUart1[39] = 0xCC;
    expectedUart1[40] = 0xDD;
    expectedUart1[41] = '\r'; // End of message frame
    expectedUart1[42] = '\n';

    // construst msg2 expect from buffer
    memcpy(expectedUart2, expectedUart1, PCDEBUG_MSG_LOG_MSGLEN);
    memset(expectedUart2 + 5, 0U, PCDEBUG_MSG_LOG_DATALEN); // clear data
    size_t remainingBytes = sizeof(longMsg) - 32;
    memcpy(expectedUart2 + 5, longMsg + 32, remainingBytes);

    Log_Print(&testLog, longMsg);

    // Stepping through to 100ms should trigger transmission
    for (uint16_t i = 0; i < 10; ++i) {
        mockSetTaskNotifyValue(1); // to wake up
        PCDebug_TaskMethod(&mPCDebug);
    }

    // 1st message should be on UART now, with second message queued
    TEST_ASSERT_EQUAL(PCDEBUG_MSG_LOG_MSGLEN, mockGet_HAL_UART_Len());
    TEST_ASSERT_EQUAL(PCDEBUG_MSG_LOG_MSGLEN, mockGetStreamBufferLen(usart1.txPendingStreamHandle));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedUart1, mockGet_HAL_UART_Data(), PCDEBUG_MSG_LOG_MSGLEN);

    // Prompt UART to send next message...
    mockClear_HAL_UART_Data();
    HAL_UART_TxCpltCallback(&husart1);
    mockClearStreamBufferData(usart1.txPendingStreamHandle);

    // 2nd message should be on UART now, with no further queued messages
    TEST_ASSERT_EQUAL(PCDEBUG_MSG_LOG_MSGLEN, mockGet_HAL_UART_Len());
    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen(usart1.txPendingStreamHandle));
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedUart2, mockGet_HAL_UART_Data(), PCDEBUG_MSG_LOG_MSGLEN);
}

TEST(DEVICE_PCDEBUG, TestSerialRecv)
{
    uint8_t recvBytes[] = {0x01, 0x2, 0xAA, 0xBB};
    uint16_t recvBytesLen = sizeof(recvBytes);

    const char expectedLog[] = "Recevied serial bytes: 0x01 0x02 0xaa 0xbb \n";
    size_t expectedLogLen = sizeof(expectedLog) - 1U; // -1 because logging skips null char

    // Nothing should be printed before there is data...
    mockSetTaskNotifyValue(1); // to wake up
    PCDebug_TaskMethod(&mPCDebug);
    TEST_ASSERT_EQUAL(0U, printfOutSize);

    // UART recieve on DMA:
    memcpy(usart1.uartDmaRx, recvBytes, recvBytesLen); // copy into DMA buffer
    HAL_UARTEx_RxEventCallback(&husart1, recvBytesLen);
    TEST_ASSERT_EQUAL(recvBytesLen, mockGetStreamBufferLen(mPCDebug.recvStreamHandle));

    // Driver should just log these bytes
    mockSetTaskNotifyValue(1); // to wake up
    PCDebug_TaskMethod(&mPCDebug);
    TEST_ASSERT_EQUAL_STRING(expectedLog, printfOut);
    TEST_ASSERT_EQUAL(expectedLogLen, printfOutSize);
}

TEST_GROUP_RUNNER(DEVICE_PCDEBUG)
{
    RUN_TEST_CASE(DEVICE_PCDEBUG, InitOk);
    RUN_TEST_CASE(DEVICE_PCDEBUG, InitTaskRegisterError);
    RUN_TEST_CASE(DEVICE_PCDEBUG, TestNoMessages);
    RUN_TEST_CASE(DEVICE_PCDEBUG, TestLogSerialShortMsg);
    RUN_TEST_CASE(DEVICE_PCDEBUG, TestLogSerialLongMsg);
    RUN_TEST_CASE(DEVICE_PCDEBUG, TestSerialRecv);
}
