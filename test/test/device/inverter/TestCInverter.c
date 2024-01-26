/**
 * TestInverter.c
 * 
 *  Created on: Mar 29 2023
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>

// Mocks for code under test (replaces stubs)
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semaphore.h"

#include "time/tasktimer/MockTasktimer.h" // Needed for vehicle state
#include "lib/logging/MockLogging.h"

#include "vehicleInterface/vehicleState/vehicleState.h"

// source code under test
#include "device/inverter/cInverter.c"

// Parameters
static const CAN_Device_T inverterCanBus = CAN_DEV2;
static CAN_HandleTypeDef hcan = {
    .Instance = CAN2
};

// Modules
static Logging_T testLog;
static VehicleState_T testVehicleState;
static CInverter_T testInverter;

static void resetInputs(void)
{
    // TODO add support for INV_ERR pin
    // mockSet_GPIO_Asserted(testPinInvErrorGpio.GPIOx, testPinInvErrorGpio.GPIO_Pin, false);
}

static void registerGPIOs(void)
{
    // TODO add support for INV_ERR pin
    // testPinInvErrorGpio.GPIOx = &testPinGpioBankA;
    // testPinInvErrorGpio.GPIO_Pin = testPinBMSGpioPin;
    // mock_GPIO_RegisterPin(testPinInvErrorGpio.GPIOx, testPinInvErrorGpio.GPIO_Pin);
}

TEST_GROUP(DEVICE_CINVERTER);

TEST_SETUP(DEVICE_CINVERTER)
{
    registerGPIOs();
    resetInputs();

    // Init logging & task timer
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);

    // Init CAN bus
    TEST_ASSERT_EQUAL(
        CAN_STATUS_OK,
        CAN_Init(&testLog));
    TEST_ASSERT_EQUAL(
        CAN_STATUS_OK,
        CAN_Config(inverterCanBus, &hcan));

    // Init vehicle state
    TEST_ASSERT_EQUAL(
        VEHICLESTATE_STATUS_OK,
        VehicleState_Init(&testLog, &testVehicleState));

    mockLogClear();
    mockSet_HAL_CAN_AllStatus(HAL_OK);
    mockClear_HAL_CAN_TxMailboxes();
    mockClear_HAL_CAN_RxFifo();

    // Init inverter
    testInverter.canInst = inverterCanBus;
    testInverter.vehicleState = &testVehicleState;
    CInverter_Status_T status = CInverter_Init(&testLog, &testInverter);
    TEST_ASSERT_EQUAL(CINVERTER_STATUS_OK, status);

    const char* expectedLogging =
        "CInverter_Init begin\n"
        "CInverter_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST_TEAR_DOWN(DEVICE_CINVERTER)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked(testVehicleState.mutex));
    mockSet_HAL_CAN_AllStatus(HAL_OK);
}

TEST(DEVICE_CINVERTER, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_CINVERTER, InverterEnable)
{
    uint32_t expectedMsgId = 0x0C0;
    uint8_t expectedMsgDataInvDisable[] = {
        0x00, 0x00,     // Torque = 0
        0x00, 0x00,     // Speed = 0
        0x01,           // Direction = forward
        0x00,           // Enabled = false, discharge = 0, speed mode = disabled
        0x00, 0x00      // Torque limit
    };
    uint8_t expectedMsgDataInvEnable[] = {
        0x00, 0x00,     // Torque = 0
        0x00, 0x00,     // Speed = 0
        0x01,           // Direction = forward
        0x01,           // Enabled = true, discharge = 0, speed mode = disabled
        0x00, 0x00      // Torque limit
    };
    _Static_assert(sizeof(expectedMsgDataInvDisable) == 8, "Msg len");
    _Static_assert(sizeof(expectedMsgDataInvEnable) == 8, "Msg len");

    // Enabling the inverter from disabled should send a disable and then enable command
    TEST_ASSERT_EQUAL(
        CINVERTER_STATUS_OK,
        CInverter_SendInverterEnabled(&testInverter, true));

    CAN_TxHeaderTypeDef* txHeader1 = mockGet_HAL_CAN_TxHeader(0);
    CAN_TxHeaderTypeDef* txHeader2 = mockGet_HAL_CAN_TxHeader(1);
    uint8_t* dataRecv1 = mockGet_HAL_CAN_TxData(0);
    uint8_t* dataRecv2 = mockGet_HAL_CAN_TxData(1);
    TEST_ASSERT_EQUAL(2, mockGet_HAL_CAN_NumTxMailboxesInUse());
    TEST_ASSERT_EQUAL(8, txHeader1->DLC);
    TEST_ASSERT_EQUAL(8, txHeader2->DLC);
    TEST_ASSERT_EQUAL(expectedMsgId, txHeader1->StdId);
    TEST_ASSERT_EQUAL(expectedMsgId, txHeader2->StdId);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgDataInvDisable, dataRecv1, 8);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgDataInvEnable, dataRecv2, 8);

    // Enabling again won't send the disable command
    mockClear_HAL_CAN_TxMailboxes();
    TEST_ASSERT_EQUAL(
        CINVERTER_STATUS_OK,
        CInverter_SendInverterEnabled(&testInverter, true));

    CAN_TxHeaderTypeDef* txHeader3 = mockGet_HAL_CAN_TxHeader(0);
    uint8_t* dataRecv3 = mockGet_HAL_CAN_TxData(0);
    TEST_ASSERT_EQUAL(1U, mockGet_HAL_CAN_NumTxMailboxesInUse());
    TEST_ASSERT_EQUAL(8, txHeader3->DLC);
    TEST_ASSERT_EQUAL(expectedMsgId, txHeader3->StdId);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgDataInvEnable, dataRecv3, 8);

    // Disabling now should just send a disable
    mockClear_HAL_CAN_TxMailboxes();
    TEST_ASSERT_EQUAL(
        CINVERTER_STATUS_OK,
        CInverter_SendInverterEnabled(&testInverter, false));

    CAN_TxHeaderTypeDef* txHeader4 = mockGet_HAL_CAN_TxHeader(0);
    uint8_t* dataRecv4 = mockGet_HAL_CAN_TxData(0);
    TEST_ASSERT_EQUAL(1U, mockGet_HAL_CAN_NumTxMailboxesInUse());
    TEST_ASSERT_EQUAL(8, txHeader4->DLC);
    TEST_ASSERT_EQUAL(expectedMsgId, txHeader4->StdId);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgDataInvDisable, dataRecv4, 8);
}

TEST(DEVICE_CINVERTER, InverterDisable)
{
    uint32_t expectedMsgId = 0x0C0;
    uint8_t expectedMsgDataInvDisable[] = {
        0x00, 0x00,     // Torque = 0
        0x00, 0x00,     // Speed = 0
        0x01,           // Direction = forward
        0x00,           // Enabled = false, discharge = 0, speed mode = disabled
        0x00, 0x00      // Torque limit
    };
    _Static_assert(sizeof(expectedMsgDataInvDisable) == 8, "Msg len");

    // Just calling disable first should only send a disabl
    mockClear_HAL_CAN_TxMailboxes();
    TEST_ASSERT_EQUAL(
        CINVERTER_STATUS_OK,
        CInverter_SendInverterEnabled(&testInverter, false));

    CAN_TxHeaderTypeDef* txHeader = mockGet_HAL_CAN_TxHeader(0);
    uint8_t* dataRecv = mockGet_HAL_CAN_TxData(0);
    TEST_ASSERT_EQUAL(1U, mockGet_HAL_CAN_NumTxMailboxesInUse());
    TEST_ASSERT_EQUAL(8, txHeader->DLC);
    TEST_ASSERT_EQUAL(expectedMsgId, txHeader->StdId);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgDataInvDisable, dataRecv, 8);
}

TEST(DEVICE_CINVERTER, InverterEnableWhileDischarging)
{
    // setup pre-conditions
    testInverter.commandData.dischargeModeEnabled = true;

    TEST_ASSERT_EQUAL(
        CINVERTER_STATUS_ERROR_DISCHARGE,
        CInverter_SendInverterEnabled(&testInverter, true));
}

TEST(DEVICE_CINVERTER, DischargeEnable)
{
    uint32_t expectedMsgId = 0x0C0;
    uint8_t expectedMsgDataDischargeEnable[] = {
        0x00, 0x00,     // Torque = 0
        0x00, 0x00,     // Speed = 0
        0x01,           // Direction = forward
        0x02,           // Enabled = false, discharge = true, speed mode = disabled
        0x00, 0x00      // Torque limit
    };
    _Static_assert(sizeof(expectedMsgDataDischargeEnable) == 8, "Msg len");

    TEST_ASSERT_EQUAL(
        CINVERTER_STATUS_OK,
        CInverter_SendInverterDischarge(&testInverter, true));

    CAN_TxHeaderTypeDef* txHeader = mockGet_HAL_CAN_TxHeader(0);
    uint8_t* dataRecv = mockGet_HAL_CAN_TxData(0);
    TEST_ASSERT_EQUAL(1U, mockGet_HAL_CAN_NumTxMailboxesInUse());
    TEST_ASSERT_EQUAL(8, txHeader->DLC);
    TEST_ASSERT_EQUAL(expectedMsgId, txHeader->StdId);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgDataDischargeEnable, dataRecv, 8);
}

TEST(DEVICE_CINVERTER, DischargeDisable)
{
    uint32_t expectedMsgId = 0x0C0;
    uint8_t expectedMsgDataDischargeEnable[] = {
        0x00, 0x00,     // Torque = 0
        0x00, 0x00,     // Speed = 0
        0x01,           // Direction = forward
        0x00,           // Enabled = false, discharge = false, speed mode = disabled
        0x00, 0x00      // Torque limit
    };
    _Static_assert(sizeof(expectedMsgDataDischargeEnable) == 8, "Msg len");

    TEST_ASSERT_EQUAL(
        CINVERTER_STATUS_OK,
        CInverter_SendInverterDischarge(&testInverter, false));

    CAN_TxHeaderTypeDef* txHeader = mockGet_HAL_CAN_TxHeader(0);
    uint8_t* dataRecv = mockGet_HAL_CAN_TxData(0);
    TEST_ASSERT_EQUAL(1U, mockGet_HAL_CAN_NumTxMailboxesInUse());
    TEST_ASSERT_EQUAL(8, txHeader->DLC);
    TEST_ASSERT_EQUAL(expectedMsgId, txHeader->StdId);
    TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgDataDischargeEnable, dataRecv, 8);
}

TEST(DEVICE_CINVERTER, DischargeEnableInverterEnabled)
{
    // Enabling discharge when the inverter is already enabled should be disallowed

    // set up pre-conditions
    testInverter.commandData.inverterEnabled = true;

    TEST_ASSERT_EQUAL(
        CINVERTER_STATUS_ERROR_NOT_DISIABLED,
        CInverter_SendInverterDischarge(&testInverter, true));
}

TEST(DEVICE_CINVERTER, RequestTorqueInverterDisabled)
{
    TEST_ASSERT_EQUAL(
        CINVERTER_STATUS_ERROR_NOT_ENABLED,
        CInverter_SendTorqueCommand(&testInverter, 0.0f, VEHICLESTATE_INVERTER_FORWARD));
}

TEST(DEVICE_CINVERTER, RequestTorqueValid)
{
    // set up pre-conditions
    testInverter.commandData.inverterEnabled = true;

    // test a variety of torque options
    uint32_t expectedMsgId = 0x0C0;
    uint8_t expectedMsgData[] = {
        0x00, 0x00,     // Torque = 0
        0x00, 0x00,     // Speed = 0
        0x01,           // Direction = forward
        0x01,           // Enabled = true, discharge = 0, speed mode = disabled
        0x00, 0x00      // Torque limit
    };
    _Static_assert(sizeof(expectedMsgData) == 8, "Msg len");

    float requests[] = {0.0f, 3276.7f, 1.0f, 0.1f, 100.0f, 150.2f, 150.27f, 1.00004f};
    size_t nRequests = sizeof(requests) / sizeof(float);

    for (size_t i = 0; i < nRequests; ++i) {
        float torqueRequest = requests[i];

        // Update torque part of CAN message
        int asInt = (int)(10.0f * torqueRequest);
        expectedMsgData[0] = (uint8_t)(asInt) & 0xFF; // little endian
        expectedMsgData[1] = (uint8_t)(asInt >> 8) & 0xFF;

        mockClear_HAL_CAN_TxMailboxes();
        TEST_ASSERT_EQUAL(
            CINVERTER_STATUS_OK,
            CInverter_SendTorqueCommand(&testInverter, torqueRequest, VEHICLESTATE_INVERTER_FORWARD));

        CAN_TxHeaderTypeDef* txHeader = mockGet_HAL_CAN_TxHeader(0);
        uint8_t* dataRecv = mockGet_HAL_CAN_TxData(0);
        TEST_ASSERT_EQUAL(1U, mockGet_HAL_CAN_NumTxMailboxesInUse());
        TEST_ASSERT_EQUAL(8, txHeader->DLC);
        TEST_ASSERT_EQUAL(expectedMsgId, txHeader->StdId);
        TEST_ASSERT_EQUAL_UINT8_ARRAY(expectedMsgData, dataRecv, 8);
    }
}

TEST(DEVICE_CINVERTER, RequestTorqueInvalid)
{
    // set up pre-conditions
    testInverter.commandData.inverterEnabled = true;

    // test a variety of torque options
    float requests[] = {-1.0f, -0.1f, 5000.0f, 3276.8f};
    size_t nRequests = sizeof(requests) / sizeof(float);

    for (size_t i = 0; i < nRequests; ++i) {
        float torqueRequest = requests[i];

        TEST_ASSERT_EQUAL(
            CINVERTER_STATUS_ERROR_VALUE,
            CInverter_SendTorqueCommand(&testInverter, torqueRequest, VEHICLESTATE_INVERTER_FORWARD));
    }
}

TEST(DEVICE_CINVERTER, RecvTemperatures1)
{
    uint32_t recvId = 0x0A0;
    uint8_t recvMsg[] = {
        0xDD, 0x01, // Module A temp: 47.7 C
        0xE8, 0x03, // Module B temp: 100.0 C
        0x00, 0x00, // Module C temp: 0.0 C
        0xFF, 0x7F  // Gate Driver board temp: 3276.7 C
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL_FLOAT(47.7f, testVehicleState.data.inverter.moduleATemperature);
    TEST_ASSERT_EQUAL_FLOAT(100.0f, testVehicleState.data.inverter.moduleBTemperature);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, testVehicleState.data.inverter.moduleCTemperature);
    TEST_ASSERT_EQUAL_FLOAT(3276.7f, testVehicleState.data.inverter.gateDriverTemp);
}

TEST(DEVICE_CINVERTER, RecvTemperatures2)
{
    uint32_t recvId = 0x0A1;
    uint8_t recvMsg[] = {
        0xCE, 0x04, // Control board temp: 123.0 C
        0x00, 0x00, // Unused
        0x00, 0x00, // Unused
        0x00, 0x00  // Unused
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL_FLOAT(123.0f, testVehicleState.data.inverter.controlBoardTemp);
}

TEST(DEVICE_CINVERTER, RecvTemperatures3)
{
    uint32_t recvId = 0x0A2;
    uint8_t recvMsg[] = {
        0x00, 0x00, // Unused
        0x00, 0x00, // Unused
        0x61, 0x02, // Motor temperature: 60.9 C
        0x00, 0x00  // Unused
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL_FLOAT(60.9f, testVehicleState.data.motor.temperature);
}

TEST(DEVICE_CINVERTER, RecvMotorPositionInformation)
{
    uint32_t recvId = 0x0A5;
    uint8_t recvMsg[] = {
        0x4F, 0x07, // Motor angle (electrical) = 187.1 deg
        0x80, 0x0C, // Motor speed = 3200 rpm
        0x81, 0x0C, // Electrical output frequency = 320.1 Hz
        0x00, 0x00  // Unused
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL_FLOAT(187.1f, testVehicleState.data.motor.angle);
    TEST_ASSERT_EQUAL(3200, testVehicleState.data.motor.speed);
    TEST_ASSERT_EQUAL_FLOAT(320.1f, testVehicleState.data.inverter.outputFrequency);
}

TEST(DEVICE_CINVERTER, RecvCurrentInformation)
{
    uint32_t recvId = 0x0A6;
    uint8_t recvMsg[] = {
        0x2F, 0x04, // Phase A current = 107.1 A
        0x00, 0x00, // Phase B current = 0 A
        0xAD, 0x01, // Phase C current = 42.9 A
        0xD5, 0x07  // DC bus current = 200.5 A
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL_FLOAT(107.1f, testVehicleState.data.motor.phaseACurrent);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, testVehicleState.data.motor.phaseBCurrent);
    TEST_ASSERT_EQUAL_FLOAT(42.9f, testVehicleState.data.motor.phaseCCurrent);
    TEST_ASSERT_EQUAL_FLOAT(200.5f, testVehicleState.data.inverter.dcBusCurrent);
}

TEST(DEVICE_CINVERTER, RecvVoltageInformation)
{
    uint32_t recvId = 0x0A7;
    uint8_t recvMsg[] = {
        0x2F, 0x04, // DC Bus voltage = 107.1 V
        0x00, 0x00, // Output voltage = 0 V
        0xAD, 0x01, // VAB_Vd_Voltage = 42.9 V
        0xD5, 0x07  // VBC_Vq_Voltage = 200.5 V
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL_FLOAT(107.1f, testVehicleState.data.inverter.dcBusVoltage);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, testVehicleState.data.inverter.outputVoltage);
    TEST_ASSERT_EQUAL_FLOAT(42.9f, testVehicleState.data.inverter.vd);
    TEST_ASSERT_EQUAL_FLOAT(200.5f, testVehicleState.data.inverter.vq);
}

TEST(DEVICE_CINVERTER, RecvFluxInformation)
{
    uint32_t recvId = 0x0A8;
    uint8_t recvMsg[] = {
        0xD6, 0x29, // Flux command = 10.71 Weber
        0x00, 0x00, // Flux feedback = 0 Weber
        0xAD, 0x01, // Id feedback = 42.9 A
        0xD5, 0x07  // Iq feedback = 200.5 A
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL_FLOAT(10.71f, testVehicleState.data.inverter.fluxCommand);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, testVehicleState.data.inverter.fluxFeedback);
    TEST_ASSERT_EQUAL_FLOAT(42.9f, testVehicleState.data.inverter.idFeedback);
    TEST_ASSERT_EQUAL_FLOAT(200.5f, testVehicleState.data.inverter.iqFeedback);
}

TEST(DEVICE_CINVERTER, RecvInternalStates)
{
    uint32_t recvId = 0x0AA;
    uint8_t recvMsg[] = {
        0x06,   // VSM State = vehicle running state
        0x00,   // Not used (PWM frequency)
        0x03,   // Inverter state = closed loop state
        0x00,   // Not used (relay state)
        0x60,   // run mode = torque, discharge state = discharge actively occurring,
        0x00,   // unused
        0x01,   // inverter enabled
        0x01,   // direction = forward
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL(VEHICLESTATE_INVERTERVSMSTATE_MOTORRUNNING, testVehicleState.data.inverter.vsmState);
    TEST_ASSERT_EQUAL(VEHICLESTATE_INVERTERSTATE_CLOSEDLOOP, testVehicleState.data.inverter.inverterState);
    TEST_ASSERT_EQUAL(VEHICLESTATE_DISCHARGESTATE_ACTIVE, testVehicleState.data.inverter.dischargeState);
    TEST_ASSERT_EQUAL(VEHICLESTATE_INVERTER_ENABLED, testVehicleState.data.inverter.enabled);
    TEST_ASSERT_EQUAL(VEHICLESTATE_INVERTER_FORWARD, testVehicleState.data.inverter.direction);
}

TEST(DEVICE_CINVERTER, RecvFaultCodes)
{
    uint32_t recvId = 0x0AB;
    uint8_t recvMsg[] = {
        0xC4, 0x3B, 0xD6, 0x47, // POST fault = 0x47d63bc4
        0xC9, 0x06, 0x94, 0x20, // RUN fault = 0x209406c9
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL(0x47d63bc4, testVehicleState.data.inverter.postFaults);
    TEST_ASSERT_EQUAL(0x209406c9, testVehicleState.data.inverter.runFaults);
}

TEST(DEVICE_CINVERTER, RecvTorqueTimerInformation)
{
    uint32_t recvId = 0x0AC;
    uint8_t recvMsg[] = {
        0x61, 0x09, // Commanded torque = 240.1 Nm
        0x97, 0x08, // Torque feedback = 219.9 Nm
        0xD5, 0x89, 0x0B, 0x00 // Power on timer = 756181 counts = 2,268.543 sec
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL_FLOAT(240.1f, testVehicleState.data.inverter.commandedTorque);
    TEST_ASSERT_EQUAL_FLOAT(219.9f, testVehicleState.data.motor.calculatedTorque);
    TEST_ASSERT_EQUAL_FLOAT(756181, testVehicleState.data.inverter.timerCounts);
}

TEST(DEVICE_CINVERTER, RecvModulationIndexFluxWeakingInformation)
{
    uint32_t recvId = 0x0AD;
    uint8_t recvMsg[] = {
        0x62, 0x00, // Modulation index = 0.98
        0x91, 0x01, // Flux weakening output = 40.1 A
        0x0B, 0xFE, // Id command = -50.1 A
        0xE1, 0x07  // Iq command = 201.7 A
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    // mockClearQueueData(canInstances[CAN_DEV2].txQueueHandle);
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testInverter.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    InverterProcessing(&testInverter);

    TEST_ASSERT_EQUAL_FLOAT(0.98f, testVehicleState.data.inverter.modulationIndex);
    TEST_ASSERT_EQUAL_FLOAT(40.1f, testVehicleState.data.inverter.fluxWeakeningOutput);
    TEST_ASSERT_EQUAL_FLOAT(-50.1f, testVehicleState.data.inverter.idCommand);
    TEST_ASSERT_EQUAL_FLOAT(201.7f, testVehicleState.data.inverter.iqCommand);
}

TEST_GROUP_RUNNER(DEVICE_CINVERTER)
{
    RUN_TEST_CASE(DEVICE_CINVERTER, InitOk);
    RUN_TEST_CASE(DEVICE_CINVERTER, InverterEnable);
    RUN_TEST_CASE(DEVICE_CINVERTER, InverterDisable);
    RUN_TEST_CASE(DEVICE_CINVERTER, InverterEnableWhileDischarging);
    RUN_TEST_CASE(DEVICE_CINVERTER, DischargeEnable);
    RUN_TEST_CASE(DEVICE_CINVERTER, DischargeDisable);
    RUN_TEST_CASE(DEVICE_CINVERTER, DischargeEnableInverterEnabled);
    RUN_TEST_CASE(DEVICE_CINVERTER, RequestTorqueInverterDisabled);
    RUN_TEST_CASE(DEVICE_CINVERTER, RequestTorqueValid);
    RUN_TEST_CASE(DEVICE_CINVERTER, RequestTorqueInvalid);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvTemperatures1);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvTemperatures2);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvTemperatures3);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvMotorPositionInformation);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvCurrentInformation);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvVoltageInformation);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvFluxInformation);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvInternalStates);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvFaultCodes);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvTorqueTimerInformation);
    RUN_TEST_CASE(DEVICE_CINVERTER, RecvModulationIndexFluxWeakingInformation);
}

#define INVOKE_TEST DEVICE_CINVERTER
#include "test_main.h"
