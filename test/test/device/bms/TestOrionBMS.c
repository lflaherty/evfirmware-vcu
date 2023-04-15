/**
 * TestOrionBMS.c
 * 
 *  Created on: 15 Apr 2023
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
#include "device/bms/orionBms.c"

// Parameters
static const CAN_Device_T inverterCanBus = CAN_DEV3;
static CAN_HandleTypeDef hcan = {
    .Instance = CAN3
};

// Modules
static Logging_T testLog;
static VehicleState_T testVehicleState;
static BMS_T testBms;

TEST_GROUP(DEVICE_ORIONBMS);

TEST_SETUP(DEVICE_ORIONBMS)
{
    mockSemaphoreSetLocked(false);

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
    testBms.canInst = inverterCanBus;
    testBms.vehicleState = &testVehicleState;
    BMS_Status_T status = BMS_Init(&testLog, &testBms);
    TEST_ASSERT_EQUAL(BMS_STATUS_OK, status);

    const char* expectedLogging =
        "BMS_Init begin\n"
        "BMS_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());
}

TEST_TEAR_DOWN(DEVICE_ORIONBMS)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());
    mockSet_HAL_CAN_AllStatus(HAL_OK);
}

TEST(DEVICE_ORIONBMS, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_ORIONBMS, RecvMaxCellState)
{
    uint32_t recvId = 0xB01;
    uint8_t recvMsg[] = {
        0x38, 0x02, // Max cell temp = 56.8 deg
        0x21,       // Max cell temp ID = 33
        0x3D, 0x01, // Max cell voltage = 3.17 V
        0x13,       // Max cell voltage ID = 19
        0x00, 0x00
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testBms.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    BMSProcessing(&testBms);

    TEST_ASSERT_EQUAL_FLOAT(56.8f, testVehicleState.data.battery.maxCellTemperature);
    TEST_ASSERT_EQUAL(33, testVehicleState.data.battery.maxCellTemperatureCellID);
    TEST_ASSERT_EQUAL_FLOAT(3.17f, testVehicleState.data.battery.maxCellVoltage);
    TEST_ASSERT_EQUAL(19, testVehicleState.data.battery.maxCellVoltageCellID);
}

TEST(DEVICE_ORIONBMS, RecvMinCellState)
{
    uint32_t recvId = 0xB02;
    uint8_t recvMsg[] = {
        0x01, 0x00, // Min cell temp = 0.1 deg
        0x00,       // Min cell temp ID = 0
        0x11, 0x00, // Min cell voltage = 0.17 V
        0xFF,       // Min cell voltage ID = 255
        0x00, 0x00
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testBms.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    BMSProcessing(&testBms);

    TEST_ASSERT_EQUAL_FLOAT(0.1f, testVehicleState.data.battery.minCellTemperature);
    TEST_ASSERT_EQUAL(0, testVehicleState.data.battery.minCellTemperatureCellID);
    TEST_ASSERT_EQUAL_FLOAT(0.17f, testVehicleState.data.battery.minCellVoltage);
    TEST_ASSERT_EQUAL(255, testVehicleState.data.battery.minCellVoltageCellID);
}

TEST(DEVICE_ORIONBMS, RecvPackState)
{
    uint32_t recvId = 0xB03;
    uint8_t recvMsg[] = {
        0xC3, 0x0B, // DC current = 301.1 A
        0x4A, 0x18, // DC voltage = 621.8 V
        0x31, 0x21, // SOC = 84.97%
        0x00, 0x00
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testBms.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    BMSProcessing(&testBms);

    TEST_ASSERT_EQUAL_FLOAT(301.1f, testVehicleState.data.battery.dcCurrent);
    TEST_ASSERT_EQUAL_FLOAT(621.8f, testVehicleState.data.battery.dcVoltage);
    TEST_ASSERT_EQUAL_FLOAT(84.97f, testVehicleState.data.battery.stateOfCarge);
}

TEST(DEVICE_ORIONBMS, RecvCounter)
{
    uint32_t recvId = 0xB04;
    uint8_t recvMsg[] = {
        0xF0, 0x4F, 0x67, 0xB9, // arbitrary counter value
        0x00, 0x00, 0x00, 0x00
    };
    _Static_assert(sizeof(recvMsg) == 8, "Msg len");

    // invoke CAN message receive
    mockAddHALCANRxMessage(recvId, recvMsg, 8);
    HAL_CAN_RxFifo0MsgPendingCallback(&hcan);
    TEST_ASSERT_EQUAL(1U * sizeof(CAN_DataFrame_T), mockGetQueueSize(testBms.canDataQueueHandle));

    // Run inverter task
    mockSetTaskNotifyValue(1); // to wake up
    BMSProcessing(&testBms);

    TEST_ASSERT_EQUAL_FLOAT(3110555632UL, testVehicleState.data.battery.bmsCounter);
}

TEST_GROUP_RUNNER(DEVICE_ORIONBMS)
{
    RUN_TEST_CASE(DEVICE_ORIONBMS, InitOk);
    RUN_TEST_CASE(DEVICE_ORIONBMS, RecvMaxCellState);
    RUN_TEST_CASE(DEVICE_ORIONBMS, RecvMinCellState);
    RUN_TEST_CASE(DEVICE_ORIONBMS, RecvPackState);
    RUN_TEST_CASE(DEVICE_ORIONBMS, RecvCounter);
}

#define INVOKE_TEST DEVICE_ORIONBMS
#include "test_main.h"
