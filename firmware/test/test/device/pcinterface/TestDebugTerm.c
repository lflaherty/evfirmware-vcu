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

#include "tasktimer/MockTasktimer.h"
#include "vehicleInterface/vehicleControl/MockVehicleControl.h"
// MockLogging.h is deliberately not used here - need the stream internals of
// logging to work correctly. Use MockStdio to capture SWO printfs instead

// hack to deal with the UART scheduling
// don't want to deal with that in this test, that's covered by TestUart
#include "uart/uart.h"
#undef UART_MAX_DMA_LEN
#define UART_MAX_DMA_LEN 65535

// source code under test
#include "uart/uart.c"
#include "device/pcinterface/pcinterface.c"
#include "device/pcinterface/periodicupdates.c"
#include "device/pcinterface/requests.c"
#include "device/pcinterface/debugterm.c"
#include "device/pcinterface/debugtermcommands.c"
#include "logging/logging.c"  // also need this to use mock impls

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

#define TEST_BUF_LEN 65535
static uint8_t dataBuf[TEST_BUF_LEN] = { 0 };

/**
 * @brief Continuously copy UART tx data out to buffer and invoke UART callback
 * until all data is received.
 * 
 * @param buffer 
 * @param bufferSize 
 * @return size_t 
 */
static size_t flushSerialData(uint8_t* buffer, const size_t bufferSize)
{
    size_t bufferIdx = 0;
    while (mockGet_HAL_UART_Len() > 0) {
        size_t chunkSize = mockGet_HAL_UART_Len();
        uint8_t* chunkData = mockGet_HAL_UART_Data();

        assert(bufferSize >= chunkSize);
        assert(bufferSize - chunkSize >= bufferIdx);

        memcpy(buffer + bufferIdx, chunkData, chunkSize);
        bufferIdx += chunkSize;

        mockClear_HAL_UART_Data();
        HAL_UART_TxCpltCallback(&husartA);
    }

    return bufferIdx;
}

static size_t expectDebugLogMsg(const char* msg, const uint8_t* buf, const size_t bufLen)
{
    size_t msgOffset = 0;
    size_t bufOffset = 0;
    size_t msgLenRemaining = strnlen(msg, DEBUGTERM_MAX_MSG_LEN);
    while (msgLenRemaining > 0) {
        TEST_ASSERT_GREATER_OR_EQUAL(bufOffset + PCINTERFACE_MSG_PACKET_BYTES, bufLen);

        // Obtain packet
        TEST_ASSERT_EQUAL(':', buf[bufOffset+0]);
        TEST_ASSERT_EQUAL(PCINTERFACE_MSG_DEBUG_TERMINAL, buf[bufOffset+4]);

        uint16_t payloadLen = 0;
        payloadLen |= (uint16_t)buf[bufOffset+5];
        payloadLen |= (uint16_t)(buf[bufOffset+6] << 8);
        
        TEST_ASSERT_EQUAL('\r', buf[bufOffset+7+payloadLen+4]);
        TEST_ASSERT_EQUAL('\n', buf[bufOffset+7+payloadLen+5]);

        // This should be a debug log message
        TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(payloadLen, msgLenRemaining, "Received less characters than expected");
        TEST_ASSERT_GREATER_OR_EQUAL(payloadLen, bufLen);
        TEST_ASSERT_EQUAL_STRING_LEN(msg + msgOffset, buf + bufOffset + 7, payloadLen);

        uint16_t packetLen = payloadLen + 13; // 11 header bytes + 2 payload len bytes
        bufOffset += packetLen;
        msgOffset += payloadLen;
        msgLenRemaining -= payloadLen;
    }

    return bufOffset;
}

/**
 * @brief Constructs debug serial messages and sets the payload to the given
 * command.
 * Works through UART implementation to load the command into the UART driver.
 * 
 * @param cmd Command string, including "\\n".
 * @param cmdLen Length of cmd string.
 */
static void sendCmdStrToSerial(char* cmd, int cmdLen)
{
    // Set the CRC that the "hardware" calculates
    mockSet_CRC(0x12345678);

    // Construct packet with empy payload
    uint8_t testMsg[] = {
        ':',       // Start
        0x00, 0x01, // Receiver addr: VCU
        0x00, 0x09, // Function: Debug term
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Payload (will populate this further down)
        0x12, 0x34, 0x56, 0x78, // CRC
        '\r', '\n'
    };
    uint16_t testMsgLen = sizeof(testMsg)*sizeof(uint8_t);
    _Static_assert(sizeof(testMsg) == PCINTERFACE_MSG_COMMON_MSGLEN, "message length");

    for (int i = 0; i < cmdLen; i += 8) {
        // copy the cmd to a set of uart messages
        memcpy(testMsg + 5, cmd + i, 8);

        // UART recieve on DMA:
        memcpy(interfaces[UART_DEV1].uartDmaRx, testMsg, testMsgLen); // copy into DMA buffer
        HAL_UARTEx_RxEventCallback(&husartA, testMsgLen);
    }
}

TEST_GROUP(DEVICE_PCINTERFACE_DEBUGTERM);

TEST_SETUP(DEVICE_PCINTERFACE_DEBUGTERM)
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

TEST_TEAR_DOWN(DEVICE_PCINTERFACE_DEBUGTERM)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(mPCInterface.mutex));
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(mVehicleState.mutex));
    // UART should always leave IRQ enabled after use
    TEST_ASSERT_TRUE(mockGet_HAL_Cortex_IRQEnabled(configUartA.txIrq));
    TEST_ASSERT_TRUE(mockGet_HAL_Cortex_IRQEnabled(configUartB.txIrq));
    mockClear_HAL_UART_Data();
    for (int i = 0; i < UART_NUM_INTERFACES; ++i) {
        if (interfaces[i].txPendingStreamHandle) {
            mockClearStreamBufferData(interfaces[i].txPendingStreamHandle);
        }
    }
}

TEST(DEVICE_PCINTERFACE_DEBUGTERM, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_PCINTERFACE_DEBUGTERM, DebugPrintMethod)
{
    const char* msg1 = "Simple message\n";
    DebugPrint(&mPCInterface, msg1);

    size_t len = flushSerialData(dataBuf, TEST_BUF_LEN);
    expectDebugLogMsg(msg1, dataBuf, len);

    // The serial messages have a payload of 32 bytes, so have a string longer than that:
    const char* msg2 = "This is a longer message that must be more than 32 bytes\n";
    TEST_ASSERT_GREATER_THAN_MESSAGE(32, strlen(msg2), "Message must stimulate 32 byte limit");

    mockClear_HAL_UART_Data();
    DebugPrint(&mPCInterface, msg2);
    HAL_UART_TxCpltCallback(&husartA);

    len = flushSerialData(dataBuf, TEST_BUF_LEN);
    expectDebugLogMsg(msg2, dataBuf, len);
}

TEST(DEVICE_PCINTERFACE_DEBUGTERM, CmdHelp)
{
    // Actual message to send
    char cmd[64] = {0};
    int cmdLen;

    cmdLen = snprintf(cmd, 64, "help\n");
    sendCmdStrToSerial(cmd, cmdLen);

    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);

    const char* expected1 = "Available commands:\n"
        "help         Display command help screen\n"
        "setpdm       Control PDM channels\n";
    size_t len = flushSerialData(dataBuf, TEST_BUF_LEN);
    expectDebugLogMsg(expected1, dataBuf, len);
}

TEST(DEVICE_PCINTERFACE_DEBUGTERM, CmdSetPdm)
{
    char cmd[64];
    int cmdLen;
    size_t responseLen;

    // Help text
    const char* expectedHelpText = "Control PDM channels\n"
        "Usage: setpdm 1 2 3 4 5 6\n"
        "1-6 are the on off states for channels 1-6, respectively.\n"
        "For example, `setpdm 0 0 1 0 1 0` turns on channels 3 and 5, and switches\n"
        "off remaining channels.\n";
    cmdLen = snprintf(cmd, 64, "help setpdm\n");
    sendCmdStrToSerial(cmd, cmdLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);

    responseLen = flushSerialData(dataBuf, TEST_BUF_LEN);
    expectDebugLogMsg(expectedHelpText, dataBuf, responseLen);

    // Actual message to send
    cmdLen = snprintf(cmd, 64, "setpdm 0 0 0 0 0 0\n");
    sendCmdStrToSerial(cmd, cmdLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);
    for (uint8_t i = 0; i < 6; ++i) {
        TEST_ASSERT_FALSE(mockGet_VehicleControl_PDMChannel(i));
    }

    // Test each channel on
    bool expectedPdmState[6] = {0, 0, 0, 0, 0, 0};
    for (uint8_t i = 0; i < 6; ++i) {
        // only turn on one channel
        memset(expectedPdmState, 0, sizeof(expectedPdmState));
        expectedPdmState[i] = true;

        cmdLen = snprintf(cmd, 64, "setpdm %u %u %u %u %u %u\n",
            expectedPdmState[0],
            expectedPdmState[1],
            expectedPdmState[2],
            expectedPdmState[3],
            expectedPdmState[4],
            expectedPdmState[5]);
        sendCmdStrToSerial(cmd, cmdLen);
        mockSetTaskNotifyValue(1); // to wake up
        PCInterface_TaskMethod(&mPCInterface);

        TEST_ASSERT_TRUE(mockGet_VehicleControl_PDMChannel(i));
        for (uint8_t j = 0; j < 6; ++j) {
            // check that every channel other than i is false
            if (i == j)
                continue;
            TEST_ASSERT_FALSE(mockGet_VehicleControl_PDMChannel(j));
        }
    }

    // And toggle it all back off...
    cmdLen = snprintf(cmd, 64, "setpdm 0 0 0 0 0 0\n");
    sendCmdStrToSerial(cmd, cmdLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);
    for (uint8_t i = 0; i < 6; ++i) {
        TEST_ASSERT_FALSE(mockGet_VehicleControl_PDMChannel(i));
    }
}

TEST(DEVICE_PCINTERFACE_DEBUGTERM, CmdSetSdc)
{
    char cmd[64];
    int cmdLen;
    size_t responseLen;

    // Help text
    const char* expectedHelpText = "Control shutdown circuit output state\n"
        "Usage: setsdc x\n"
        "where x is 0 or 1 to disable or enable the SDC output.\n";
    cmdLen = snprintf(cmd, 64, "help setsdc\n");
    sendCmdStrToSerial(cmd, cmdLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);

    responseLen = flushSerialData(dataBuf, TEST_BUF_LEN);
    expectDebugLogMsg(expectedHelpText, dataBuf, responseLen);

    // Actual message to send
    cmdLen = snprintf(cmd, 64, "setsdc 0\n");
    sendCmdStrToSerial(cmd, cmdLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);
    TEST_ASSERT_FALSE(mockGet_VehicleControl_ECUError());

    // Toggle error on and run again
    cmdLen = snprintf(cmd, 64, "setsdc 1\n");
    sendCmdStrToSerial(cmd, cmdLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);
    TEST_ASSERT_TRUE(mockGet_VehicleControl_ECUError());

    // And toggle it back off...
    cmdLen = snprintf(cmd, 64, "setsdc 0\n");
    sendCmdStrToSerial(cmd, cmdLen);
    mockSetTaskNotifyValue(1); // to wake up
    PCInterface_TaskMethod(&mPCInterface);
    TEST_ASSERT_FALSE(mockGet_VehicleControl_ECUError());
}

TEST_GROUP_RUNNER(DEVICE_PCINTERFACE_DEBUGTERM)
{
    RUN_TEST_CASE(DEVICE_PCINTERFACE_DEBUGTERM, InitOk);
    RUN_TEST_CASE(DEVICE_PCINTERFACE_DEBUGTERM, DebugPrintMethod);
    RUN_TEST_CASE(DEVICE_PCINTERFACE_DEBUGTERM, CmdHelp);
    RUN_TEST_CASE(DEVICE_PCINTERFACE_DEBUGTERM, CmdSetPdm);
    RUN_TEST_CASE(DEVICE_PCINTERFACE_DEBUGTERM, CmdSetSdc);
}

#define INVOKE_TEST DEVICE_PCINTERFACE_DEBUGTERM
#include "test_main.h"
