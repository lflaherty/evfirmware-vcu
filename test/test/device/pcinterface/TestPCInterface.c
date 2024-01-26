/**
 * TestPCInterface.c
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
#include "vehicleInterface/vehicleControl/MockVehicleControl.h"
// MockLogging.h is deliberately not used here - need the stream internals of
// logging to work correctly. Use MockStdio to capture SWO printfs instead

// hack to deal with the UART scheduling
// don't want to deal with that in this test, that's covered by TestUart
#include "comm/uart/uart.h"
#undef UART_MAX_DMA_LEN
#define UART_MAX_DMA_LEN 65535

// source code under test
#include "comm/uart/uart.c"
#include "device/pcinterface/pcinterface.c"
#include "device/pcinterface/periodicupdates.c"
#include "device/pcinterface/requests.c"
#include "device/pcinterface/debugterm.c"
#include "device/pcinterface/debugtermcommands.c"
#include "lib/logging/logging.c"  // also need this to use mock impls

static Logging_T testLog;

// Pins
static GPIO_TypeDef gpioBankA;
static GPIO_T mPinToggle = { .GPIOx = &gpioBankA, .GPIO_Pin = 1U };

static CRC_HandleTypeDef hcrc;
static CRC_T mCrc;
static UART_HandleTypeDef husartA = {
    .Instance = USART1,
};
static UART_HandleTypeDef husartB = {
    .Instance = USART3,
};
static UART_DeviceConfig_T configUartA = {
  .dev = UART_DEV1,
  .handle = &husartA,
  .txIrq = DMA2_Stream1_IRQn,
};
static UART_DeviceConfig_T configUartB = {
  .dev = UART_DEV3,
  .handle = &husartB,
  .txIrq = DMA2_Stream2_IRQn,
};

static VehicleState_T mVehicleState;
static CInverter_T mInverter;
static PDM_T mPdm;
static VehicleControl_T mVehicleControl;

static PCInterface_T mPCInterface;

// Number of state update messages are sent in succession
const uint32_t STATEUPDATE_NUMMSGS_SDC = 1;
const uint32_t STATEUPDATE_NUMMSGS_PDM = 1;
const uint32_t STATEUPDATE_NUMMSGS_BATTERY = 9;
const uint32_t STATEUPDATE_NUMMSGS =
        STATEUPDATE_NUMMSGS_SDC +
        STATEUPDATE_NUMMSGS_PDM +
        STATEUPDATE_NUMMSGS_BATTERY;

/**
 * @brief The state message is often the first to send - at 1Hz, but it is sent
 * on the first invocation of the task method
 * Check that it's there and then clear it so that the test can continue testing
 * what it actually cares about
 * 
 * @returns Number of bytes to skip to ignore state msg
 */
static size_t expectStateMsg(void)
{
    // The first message will be queued but once we trigger the UART to
    // tx more, in this test it will put *all* remaining queued bytes onto
    // the UART line.
    TEST_ASSERT_EQUAL(PCINTERFACE_MSG_STATEUPDATE_MSGLEN, mockGet_HAL_UART_Len());
    // Flush the first message to expose the second
    mockClear_HAL_UART_Data();
    HAL_UART_TxCpltCallback(&husartA);

    size_t remainingBytes = 0;
    if (STATEUPDATE_NUMMSGS > 1) {
        uint16_t remainingMsgs = STATEUPDATE_NUMMSGS - 1;
        remainingBytes = remainingMsgs * PCINTERFACE_MSG_STATEUPDATE_MSGLEN;

        TEST_ASSERT_GREATER_OR_EQUAL(remainingBytes, mockGet_HAL_UART_Len());
        // Flush the first message to expose the second
        // mockClear_HAL_UART_Data();
        // HAL_UART_TxCpltCallback(&husartA);
    }

    return remainingBytes;
}

TEST_GROUP(DEVICE_PCINTERFACE);

TEST_SETUP(DEVICE_PCINTERFACE)
{
    mock_GPIO_RegisterPin(mPinToggle.GPIOx, mPinToggle.GPIO_Pin);

    // Init logging
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));

    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);

    // Init CRC
    hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
    mCrc.hcrc = &hcrc;
    TEST_ASSERT_EQUAL(CRC_STATUS_OK, CRC_Init(&testLog, &mCrc));

    // Init UART
    HAL_NVIC_EnableIRQ(configUartA.txIrq);
    HAL_NVIC_EnableIRQ(configUartB.txIrq);
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_Init(&testLog));
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_Config(&configUartA));
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_Config(&configUartB));

    // Init vehicle state
    TEST_ASSERT_EQUAL(
        VEHICLESTATE_STATUS_OK,
        VehicleState_Init(&testLog, &mVehicleState));

    // Init mock inverter (required for vehicle control)
    TEST_ASSERT_EQUAL(CINVERTER_STATUS_OK, CInverter_Init(&testLog, &mInverter));
    TEST_ASSERT_EQUAL(PDM_STATUS_OK, PDM_Init(&testLog, &mPdm));

    // Init vehicle control interface
    mVehicleControl.inverter = &mInverter;
    mVehicleControl.pdm = &mPdm;
    TEST_ASSERT_EQUAL(
        VEHICLECONTROL_STATUS_OK,
        VehicleControl_Init(&testLog, &mVehicleControl));

    // clear to only capture PC debug prints
    mockClearPrintf();
    
    // Init PC Debug
    memset(&mPCInterface, 0U, sizeof(PCInterface_T));
    mPCInterface.uartA = UART_DEV1;
    mPCInterface.uartB = UART_DEV3;
    mPCInterface.crc = &mCrc;
    mPCInterface.pinToggle = &mPinToggle;

    PCInterface_Status_T status = PCInterface_Init(&testLog, &mPCInterface);
    TEST_ASSERT_EQUAL(PCINTERFACE_STATUS_OK, status);

    const char* expectedLogging =
        "PCInterface_Init begin\n"
        "PCInterface_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, printfOut);

    TEST_ASSERT_EQUAL(
        PCINTERFACE_STATUS_OK,
        PCInterface_SetVehicleState(&mPCInterface, &mVehicleState));
    TEST_ASSERT_EQUAL(
        PCINTERFACE_STATUS_OK,
        PCInterface_SetVehicleControl(&mPCInterface, &mVehicleControl));

    // clear again for coming tests (because init prints)
    mockClearStreamBufferData(mPCInterface.logStreamHandle);
    mockClearStreamBufferData(interfaces[UART_DEV1].txPendingStreamHandle);
    mockClear_HAL_UART_Data();
    mockClearPrintf();
    mockReset_VehicleControl();
}

TEST_TEAR_DOWN(DEVICE_PCINTERFACE)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(mPCInterface.mutex));
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(mVehicleState.mutex));
    // UART should always leave IRQ enabled after use
    TEST_ASSERT_TRUE(mockGet_HAL_Cortex_IRQEnabled(configUartA.txIrq));
    TEST_ASSERT_TRUE(mockGet_HAL_Cortex_IRQEnabled(configUartB.txIrq));
    mockClear_HAL_UART_Data();
}

TEST(DEVICE_PCINTERFACE, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_PCINTERFACE, InitTaskRegisterError)
{
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_ERROR_FULL);

    PCInterface_Status_T status = PCInterface_Init(&testLog, &mPCInterface);
    TEST_ASSERT(PCINTERFACE_STATUS_ERROR_INIT == status);

    const char* expectedLogging =
        "PCInterface_Init begin\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, printfOut);
}

TEST(DEVICE_PCINTERFACE, TestNoMessages)
{
    // With no log messages, should only get state update

    // First 500ms will attempt to print twice...
    for (uint16_t i = 0; i < 5U; ++i) {
        mockSetTaskNotifyValue(1); // to wake up
        PCInterface_TaskMethod(&mPCInterface);
    }

    TEST_ASSERT_EQUAL(PCINTERFACE_MSG_STATEUPDATE_MSGLEN, mockGet_HAL_UART_Len());
}

TEST(DEVICE_PCINTERFACE, TestLogSerialShortMsg)
{
    const char simpleMsg[] = "Hello!\n"; // less than 32 data chars
    assert(sizeof(simpleMsg) <= 32U);

    mockSet_CRC(0xAABBCCDD);
    const uint8_t expectedUart[PCINTERFACE_MSG_LOG_MSGLEN] = {
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
    PCInterface_TaskMethod(&mPCInterface);

    // Expecting: A state update message, and a log message
    size_t stateBytes = expectStateMsg();
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedUart, mPCInterface.mfLogDataBuffer, PCINTERFACE_MSG_LOG_MSGLEN);
    TEST_ASSERT_EQUAL(PCINTERFACE_MSG_LOG_MSGLEN, mockGet_HAL_UART_Len() - stateBytes);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedUart, mockGet_HAL_UART_Data() + stateBytes, PCINTERFACE_MSG_LOG_MSGLEN);
}

TEST(DEVICE_PCINTERFACE, TestLogSerialLongMsg)
{
    // A log message that needs to be split into two message (i.e. longer than 32)
    const char longMsg[] = "Never gonna give you up\n"
                           "Never gonna let you down\n";
    assert(sizeof(longMsg) > 32U);

    mockSet_CRC(0xAABBCCDD);
    uint8_t expectedUart1[PCINTERFACE_MSG_LOG_MSGLEN] = { 0 };
    uint8_t expectedUart2[PCINTERFACE_MSG_LOG_MSGLEN] = { 0 };

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
    memcpy(expectedUart2, expectedUart1, PCINTERFACE_MSG_LOG_MSGLEN);
    memset(expectedUart2 + 5, 0U, PCINTERFACE_MSG_LOG_DATALEN); // clear data
    size_t remainingBytes = sizeof(longMsg) - 32;
    memcpy(expectedUart2 + 5, longMsg + 32, remainingBytes);

    Log_Print(&testLog, longMsg);

    // Stepping through to 100ms should trigger transmission
    for (uint16_t i = 0; i < 10; ++i) {
        mockSetTaskNotifyValue(1); // to wake up
        PCInterface_TaskMethod(&mPCInterface);
    }

    // 1st message should be on UART now, with second message queued
    // (following the state update, because that always sends first)
    size_t stateBytes = expectStateMsg();
    // The state message is immediately sent, but the log messages are queued
    // and transmitted together after that.
    TEST_ASSERT_EQUAL(2*PCINTERFACE_MSG_LOG_MSGLEN, mockGet_HAL_UART_Len() - stateBytes);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedUart1,
            mockGet_HAL_UART_Data() + stateBytes,
            PCINTERFACE_MSG_LOG_MSGLEN);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedUart2,
            mockGet_HAL_UART_Data() + stateBytes + PCINTERFACE_MSG_LOG_MSGLEN,
            PCINTERFACE_MSG_LOG_MSGLEN);

    // Prompt UART to send next message...
    mockClear_HAL_UART_Data();
    HAL_UART_TxCpltCallback(&husartA);
    mockClearStreamBufferData(interfaces[UART_DEV1].txPendingStreamHandle);

    // 2nd message should be on UART now, with no further queued messages
    TEST_ASSERT_EQUAL(0, mockGet_HAL_UART_Len());
}

TEST(DEVICE_PCINTERFACE, PeriodicStateUpdates)
{
    // Just what the CRC hardware produces, doesn't really matter what the
    // test sets it to here.
    mockSet_CRC(0x12345678);

    // Set some SDC state to tx
    mVehicleState.data.vehicle.sdc.bspd = true;
    mVehicleState.data.vehicle.sdc.out = true;
    mVehicleState.data.glv.pdmChState[0] = true;
    mVehicleState.data.glv.pdmChState[4] = true;
    mVehicleState.data.glv.pdmChState[5] = true;

    const uint8_t expectedMsgSDC[] = {
        ':',       // Start
        0x00, 0x02, // Receiver addr
        0x00, 0x01, // Function
        0x00, 0x01, // Payload: field ID: SDC
        0x01,       // Payload: field size
        0x00, 0x00, 0x00, 0x0A, // Payload: SDC bits
        0x12, 0x34, 0x56, 0x78, // CRC
        '\r', '\n'
    };
    _Static_assert(sizeof(expectedMsgSDC) == PCINTERFACE_MSG_STATEUPDATE_MSGLEN, "State update msg length");

    const uint8_t expectedMsgPDM[] = {
        ':',       // Start
        0x00, 0x02, // Receiver addr
        0x00, 0x01, // Function
        0x00, 0x02, // Payload: field ID: PDM
        0x01,       // Payload: field size
        0x00, 0x00, 0x00, 0x31, // Payload: PDM bits
        0x12, 0x34, 0x56, 0x78, // CRC
        '\r', '\n'
    };
    _Static_assert(sizeof(expectedMsgPDM) == PCINTERFACE_MSG_STATEUPDATE_MSGLEN, "State update msg length");

    // invoke PC controller's periodic task
    for (int i = 0; i < 100; ++i) {
        mockSetTaskNotifyValue(1); // to wake up
        PCInterface_TaskMethod(&mPCInterface);
    }

    // First message
    TEST_ASSERT_EQUAL(PCINTERFACE_MSG_STATEUPDATE_MSGLEN, mockGet_HAL_UART_Len());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgSDC,
        mockGet_HAL_UART_Data(),
        PCINTERFACE_MSG_STATEUPDATE_MSGLEN);

    // Copy the pending bytes to the bus
    mockClear_HAL_UART_Data();
    HAL_UART_TxCpltCallback(&husartA);

    const size_t numMessages = STATEUPDATE_NUMMSGS - 1;
    TEST_ASSERT_EQUAL(numMessages*PCINTERFACE_MSG_STATEUPDATE_MSGLEN, mockGet_HAL_UART_Len());
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgPDM,
        mockGet_HAL_UART_Data(),
        PCINTERFACE_MSG_STATEUPDATE_MSGLEN);
}

TEST(DEVICE_PCINTERFACE, TestCommandSDC)
{
    // Set the CRC that the "hardware" calculates
    mockSet_CRC(0x12345678);

    uint8_t testMsg[] = {
        ':',       // Start
        0x00, 0x01, // Receiver addr: VCU
        0x01, 0x01, // Function: Test Cmd, Set SDC output
        0x00, 0x00, 0x00, 0x00, // Empty payload bytes
        0x00, 0x00, 0x00,       // Empty payload bytes
        0x00, // Payload: error output state
        0x12, 0x34, 0x56, 0x78, // CRC
        '\r', '\n'
    };
    uint16_t testMsgLen = sizeof(testMsg)*sizeof(uint8_t);
    _Static_assert(sizeof(testMsg) == PCINTERFACE_MSG_COMMON_MSGLEN, "message length");

    // UART recieve on DMA:
    memcpy(interfaces[UART_DEV1].uartDmaRx, testMsg, testMsgLen); // copy into DMA buffer
    HAL_UARTEx_RxEventCallback(&husartA, testMsgLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);

    TEST_ASSERT_FALSE(mockGet_VehicleControl_ECUError());

    // Toggle error on and run again
    // (note that this only works because we're faking the hardware CRC)
    testMsg[12] = 0x01;

    memcpy(interfaces[UART_DEV1].uartDmaRx, testMsg, testMsgLen); // copy into DMA buffer
    HAL_UARTEx_RxEventCallback(&husartA, testMsgLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);

    TEST_ASSERT_TRUE(mockGet_VehicleControl_ECUError());

    // And toggle it back off...
    testMsg[12] = 0x00;

    memcpy(interfaces[UART_DEV1].uartDmaRx, testMsg, testMsgLen); // copy into DMA buffer
    HAL_UARTEx_RxEventCallback(&husartA, testMsgLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);

    TEST_ASSERT_FALSE(mockGet_VehicleControl_ECUError());
}

TEST(DEVICE_PCINTERFACE, TestCommandPDM)
{
    // Set the CRC that the "hardware" calculates
    mockSet_CRC(0x12345678);

    uint8_t testMsg[] = {
        ':',       // Start
        0x00, 0x01, // Receiver addr: VCU
        0x01, 0x02, // Function: Test Cmd, Set PDM output
        0x00, 0x00, // Empty bytes
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload: PDM channels
        0x12, 0x34, 0x56, 0x78, // CRC
        '\r', '\n'
    };
    uint16_t testMsgLen = sizeof(testMsg)*sizeof(uint8_t);
    _Static_assert(sizeof(testMsg) == PCINTERFACE_MSG_COMMON_MSGLEN, "message length");

    // UART recieve on DMA:
    memcpy(interfaces[UART_DEV1].uartDmaRx, testMsg, testMsgLen); // copy into DMA buffer
    HAL_UARTEx_RxEventCallback(&husartA, testMsgLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);

    for (uint8_t i = 0; i < 6; ++i) {
        TEST_ASSERT_FALSE(mockGet_VehicleControl_PDMChannel(i));
    }

    for (uint8_t i = 0; i < 6; ++i) {
        // Toggle PDM on and run again
        // (note that this only works because we're faking the hardware CRC)
        testMsg[7] = 0x00;
        testMsg[8] = 0x00;
        testMsg[9] = 0x00;
        testMsg[10] = 0x00;
        testMsg[11] = 0x00;
        testMsg[12] = 0x00;

        testMsg[i + 7U] = 0x01; // PDM i

        memcpy(interfaces[UART_DEV1].uartDmaRx, testMsg, testMsgLen); // copy into DMA buffer
        HAL_UARTEx_RxEventCallback(&husartA, testMsgLen);
        mockSetTaskNotifyValue(1); // to wake up
        PCInterface_TaskMethod(&mPCInterface);

        TEST_ASSERT_TRUE(mockGet_VehicleControl_PDMChannel(i));
        for (uint8_t j = 0; j < 6; ++j) {
            if (i == j)
                continue;
            TEST_ASSERT_FALSE(mockGet_VehicleControl_PDMChannel(j));
        }
    }

    // And toggle it back off...
    testMsg[7] = 0x00;
    testMsg[8] = 0x00;
    testMsg[9] = 0x00;
    testMsg[10] = 0x00;
    testMsg[11] = 0x00;
    testMsg[12] = 0x00;

    memcpy(interfaces[UART_DEV1].uartDmaRx, testMsg, testMsgLen); // copy into DMA buffer
    HAL_UARTEx_RxEventCallback(&husartA, testMsgLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);

    for (uint8_t i = 0; i < 6; ++i) {
        TEST_ASSERT_FALSE(mockGet_VehicleControl_PDMChannel(i));
    }
}

TEST_GROUP_RUNNER(DEVICE_PCINTERFACE)
{
    RUN_TEST_CASE(DEVICE_PCINTERFACE, InitOk);
    RUN_TEST_CASE(DEVICE_PCINTERFACE, InitTaskRegisterError);
    RUN_TEST_CASE(DEVICE_PCINTERFACE, TestNoMessages);
    RUN_TEST_CASE(DEVICE_PCINTERFACE, TestLogSerialShortMsg);
    RUN_TEST_CASE(DEVICE_PCINTERFACE, TestLogSerialLongMsg);
    RUN_TEST_CASE(DEVICE_PCINTERFACE, PeriodicStateUpdates);
    RUN_TEST_CASE(DEVICE_PCINTERFACE, TestCommandSDC);
    RUN_TEST_CASE(DEVICE_PCINTERFACE, TestCommandPDM);
}

#define INVOKE_TEST DEVICE_PCINTERFACE
#include "test_main.h"
