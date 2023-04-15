/*
 * TestUart.c
 * 
 *  Created on: 25 Apr 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

// Mocks for code under test
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "stream_buffer.h"

#include "lib/logging/MockLogging.h"

// source code under test
#include "comm/uart/uart.c"

static Logging_T testLog;
static UART_HandleTypeDef husart1 = {
    .Instance = USART1,
};
static UART_DeviceConfig_T configUart = {
  .dev = UART_DEV1,
  .handle = &husart1,
  .rxIrq = DMA2_Stream1_IRQn,
};

TEST_GROUP(COMM_UART);

TEST_SETUP(COMM_UART)
{
    mockSet_HAL_DMA_IT_Enabled(true);
    mockSet_HAL_UART_All_Status(HAL_OK);
    mockClear_HAL_UART_Data();

    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();

    // Enable interrupts (important for test as UART code enables and disables these)
    HAL_NVIC_EnableIRQ(configUart.rxIrq);

    // Perform init
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_Init(&testLog));
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_Config(&configUart));

    TEST_ASSERT_FALSE(interfaces[UART_DEV1].txInProgress);

    const char* expectedLogging = 
        "UART_Init begin\n"
        "UART_Init complete\n"
        "UART_Config begin\n"
        "UART_Config complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for the following test
    mockLogClear();
    mockClearStreamBufferData(interfaces[UART_DEV1].txPendingStreamHandle);
}

TEST_TEAR_DOWN(COMM_UART)
{
    // UART should always leave IRQ enabled after use
    TEST_ASSERT_TRUE(mockGet_HAL_Cortex_IRQEnabled(configUart.rxIrq));
    mockLogClear();
}

TEST(COMM_UART, TestInitOk)
{
    // Done by TEST_SETUP
}

TEST(COMM_UART, TestTx)
{
    size_t streamBufferDataLen = 16;
    uint8_t streamBufferData[16] = { 0 };

    // Test data:
    uint8_t msg1[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    uint8_t msg2[] = {0x07, 0x08, 0x09, 0x0A, 0x0B};
    uint8_t msg3[] = {0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12};
    uint16_t msg1Len = sizeof(msg1);
    uint16_t msg2Len = sizeof(msg2);
    uint16_t msg3Len = sizeof(msg3);

    // internal value should start at 0
    TEST_ASSERT_FALSE(interfaces[UART_DEV1].txInProgress);

    // Send first message
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_SendMessage(UART_DEV1, msg1, msg1Len));

    // DMA transfer occurring:
    TEST_ASSERT_TRUE(interfaces[UART_DEV1].txInProgress);
    TEST_ASSERT_EQUAL(msg1Len, mockGet_HAL_UART_Len());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg1, mockGet_HAL_UART_Data(), msg1Len);
    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));

    // Allow the DMA tx to complete
    mockClear_HAL_UART_Data();
    HAL_UART_TxCpltCallback(&husart1);

    // Transfer should be complete
    TEST_ASSERT_FALSE(interfaces[UART_DEV1].txInProgress);
    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));

    // Send a second message
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_SendMessage(UART_DEV1, msg2, msg2Len));

    // DMA transfer occurring:
    TEST_ASSERT_TRUE(interfaces[UART_DEV1].txInProgress);
    TEST_ASSERT_EQUAL(msg2Len, mockGet_HAL_UART_Len());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg2, mockGet_HAL_UART_Data(), msg2Len);
    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));

    // Sending a new message while DMA is still going should enqueue it
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_SendMessage(UART_DEV1, msg3, msg3Len));
    TEST_ASSERT_TRUE(interfaces[UART_DEV1].txInProgress);
    TEST_ASSERT_EQUAL(msg2Len, mockGet_HAL_UART_Len());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg2, mockGet_HAL_UART_Data(), msg2Len);
    TEST_ASSERT_EQUAL(msg3Len, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));
    mockGetStreamBufferData(interfaces[UART_DEV1].txPendingStreamHandle, streamBufferData, streamBufferDataLen);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg3, streamBufferData, msg3Len);

    // Allow the 2nd DMA tx to complete
    mockClear_HAL_UART_Data();
    HAL_UART_TxCpltCallback(&husart1);
    mockClearStreamBufferData(interfaces[UART_DEV1].txPendingStreamHandle);

    // data in stream buffer should send now
    TEST_ASSERT_TRUE(interfaces[UART_DEV1].txInProgress);
    TEST_ASSERT_EQUAL(msg3Len, mockGet_HAL_UART_Len());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg3, mockGet_HAL_UART_Data(), msg3Len);
}

TEST(COMM_UART, TestTxFail)
{
    // Test data:
    uint8_t msg1[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    uint8_t msg2[] = {0x07, 0x08, 0x09, 0x0A, 0x0B};
    uint8_t msg3[] = {0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11, 0x12};
    uint16_t msg1Len = sizeof(msg1);
    uint16_t msg2Len = sizeof(msg2);
    uint16_t msg3Len = sizeof(msg3);

    // internal value should start at 0
    TEST_ASSERT_FALSE(interfaces[UART_DEV1].txInProgress);

    mockSet_HAL_UART_Transmit_DMA_Status(HAL_ERROR);
    TEST_ASSERT_EQUAL(UART_STATUS_ERROR_TX, UART_SendMessage(UART_DEV1, msg1, msg1Len));

    TEST_ASSERT_FALSE(interfaces[UART_DEV1].txInProgress);
    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));
    TEST_ASSERT_EQUAL(msg1Len, mockGet_HAL_UART_Len());

    // Now test queued data fails (let first message go through)
    mockSet_HAL_UART_Transmit_DMA_Status(HAL_OK);
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_SendMessage(UART_DEV1, msg2, msg2Len));

    TEST_ASSERT_TRUE(interfaces[UART_DEV1].txInProgress);
    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));
    TEST_ASSERT_EQUAL(msg2Len, mockGet_HAL_UART_Len());

    // Add the queued data
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_SendMessage(UART_DEV1, msg3, msg3Len));

    TEST_ASSERT_TRUE(interfaces[UART_DEV1].txInProgress);
    TEST_ASSERT_EQUAL(msg3Len, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));
    TEST_ASSERT_EQUAL(msg2Len, mockGet_HAL_UART_Len());

    // Handle a failed send from the ISR...
    mockClear_HAL_UART_Data();
    mockSet_HAL_UART_Transmit_DMA_Status(HAL_ERROR);
    HAL_UART_TxCpltCallback(&husart1);

    TEST_ASSERT_FALSE(interfaces[UART_DEV1].txInProgress);
}

TEST(COMM_UART, TestRx)
{
    uint8_t mockStreamBufferData[32] = { 0 };
    size_t mockStreamBufferDataLen = sizeof(mockStreamBufferData);
    // Test data:
    uint8_t msg1[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06};
    uint8_t msg2[] = {0x07, 0x08, 0x09, 0x0A, 0x0B};
    uint8_t msg1_2Combined[] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
    uint16_t msg1Len = sizeof(msg1);
    uint16_t msg2Len = sizeof(msg2);
    uint16_t msg1_2CombinedLen = sizeof(msg1_2Combined);

    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_SetRecvStream(UART_DEV1, interfaces[UART_DEV1].txPendingStreamHandle));

    TEST_ASSERT_EQUAL(0U, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));
    
    // Receive first message
    memcpy(interfaces[UART_DEV1].uartDmaRx, msg1, msg1Len); // copy into DMA buffer
    HAL_UARTEx_RxEventCallback(&husart1, msg1Len);

    TEST_ASSERT_EQUAL(msg1Len, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));
    mockGetStreamBufferData(interfaces[UART_DEV1].txPendingStreamHandle, mockStreamBufferData, mockStreamBufferDataLen);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg1, mockStreamBufferData, msg1Len);

    // Receive second message
    memcpy(interfaces[UART_DEV1].uartDmaRx, msg2, msg2Len);
    HAL_UARTEx_RxEventCallback(&husart1, msg2Len);

    TEST_ASSERT_EQUAL(msg1_2CombinedLen, mockGetStreamBufferLen(interfaces[UART_DEV1].txPendingStreamHandle));
    mockGetStreamBufferData(interfaces[UART_DEV1].txPendingStreamHandle, mockStreamBufferData, mockStreamBufferDataLen);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(msg1_2Combined, mockStreamBufferData, msg1_2CombinedLen);
}

TEST(COMM_UART, TestErrorCallback)
{
    interfaces[UART_DEV1].txInProgress = true;

    HAL_UART_ErrorCallback(&husart1);

    TEST_ASSERT_FALSE(interfaces[UART_DEV1].txInProgress);
}

TEST_GROUP_RUNNER(COMM_UART)
{
    RUN_TEST_CASE(COMM_UART, TestInitOk);
    RUN_TEST_CASE(COMM_UART, TestTx);
    RUN_TEST_CASE(COMM_UART, TestTxFail);
    RUN_TEST_CASE(COMM_UART, TestRx);
    RUN_TEST_CASE(COMM_UART, TestErrorCallback);
}

#define INVOKE_TEST COMM_UART
#include "test_main.h"
