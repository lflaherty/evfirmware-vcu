/**
 * TestGps.c
 * 
 *  Created on: Nov 7 2022
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
#include "queue.h"
#include "stream_buffer.h"
#include "semaphore.h"

#include "time/tasktimer/MockTasktimer.h"
#include "lib/logging/MockLogging.h"

#include "vehicleInterface/vehicleState/vehicleState.h"

// source code under test
#include "comm/uart/uart.c"
#include "device/gps/gps.c"

static GPIO_T pin3dFix;

static Logging_T testLog;
static UART_HandleTypeDef husart;
static VehicleState_T mVehicleState;
static GPS_T mGps;

TEST_GROUP(DEVICE_GPS);

TEST_SETUP(DEVICE_GPS)
{
    mockSemaphoreSetLocked(false);

    // Init logging
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));

    mockSet_TaskTimer_Init_Status(TASKTIMER_STATUS_OK);
    mockSet_TaskTimer_RegisterTask_Status(TASKTIMER_STATUS_OK);

    // Init UART
    husart.Instance = USART1;
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_Init(&testLog));
    TEST_ASSERT_EQUAL(UART_STATUS_OK, UART_Config(&husart));

    // Init vehicle state
    TEST_ASSERT_EQUAL(
        VEHICLESTATE_STATUS_OK,
        VehicleState_Init(&testLog, &mVehicleState));
    
    mockLogClear();
    
    // Init GPS
    memset(&mGps, 0U, sizeof(GPS_T));
    mGps.huart = &husart;
    mGps.pin3dFix = &pin3dFix;
    mGps.state = &mVehicleState;

    GPS_Status_T status = GPS_Init(&testLog, &mGps);
    TEST_ASSERT(GPS_STATUS_OK == status);

    const char* expectedLogging =
        "GPS_Init begin\n"
        "GPS_Init complete\n";
    TEST_ASSERT_EQUAL_STRING(expectedLogging, mockLogGet());

    // clear again for coming tests (because init prints)
    mockClear_HAL_UART_Data();
}

TEST_TEAR_DOWN(DEVICE_GPS)
{
    TEST_ASSERT_FALSE(mockSempahoreGetLocked());
    mockClear_HAL_UART_Data();
    mockClearStreamBufferData(mGps.recvStreamHandle);
}

TEST(DEVICE_GPS, InitOk)
{
    // Done by TEST_SETUP
}

TEST(DEVICE_GPS, GpsReceiveGPGGA)
{
    const char stringEmpty[] = "";
    const char expectedLat[] = "2236.2791";
    const char expectedLong[] = "12017.2818";
    const char msg[] = "$GPGGA,091626.042,2236.2791,N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*60\r\n";
    const uint16_t msgLen = (uint16_t)strlen(msg) * sizeof(char);

    // Empty initially
    GPS_TaskMethod(&mGps);
    TEST_ASSERT_EQUAL(0, mVehicleState.data.vehicle.gps.utcTime.hour);
    TEST_ASSERT_EQUAL(0, mVehicleState.data.vehicle.gps.utcTime.min);
    TEST_ASSERT_EQUAL(0, mVehicleState.data.vehicle.gps.utcTime.sec);
    TEST_ASSERT_EQUAL(0, mVehicleState.data.vehicle.gps.utcTime.millisec);
    TEST_ASSERT_EQUAL_STRING(stringEmpty, mVehicleState.data.vehicle.gps.latitude);
    TEST_ASSERT_EQUAL_CHAR('\0', mVehicleState.data.vehicle.gps.nsIndicator);
    TEST_ASSERT_EQUAL_STRING(stringEmpty, mVehicleState.data.vehicle.gps.longitude);
    TEST_ASSERT_EQUAL_CHAR('\0', mVehicleState.data.vehicle.gps.ewIndicator);
    TEST_ASSERT_EQUAL(0, mVehicleState.data.vehicle.gps.nSatellites);

    // Load in mock data
    memcpy(usart1.uartDmaRx, msg, msgLen); // copy into DMA buffer
    mockSet_HAL_UART_Data(msg, msgLen);
    HAL_UARTEx_RxEventCallback(&husart, msgLen);

    mockSetTaskNotifyValue(1); // to wake up
    GPS_TaskMethod(&mGps);
    TEST_ASSERT_EQUAL(9, mVehicleState.data.vehicle.gps.utcTime.hour);
    TEST_ASSERT_EQUAL(16, mVehicleState.data.vehicle.gps.utcTime.min);
    TEST_ASSERT_EQUAL(26, mVehicleState.data.vehicle.gps.utcTime.sec);
    TEST_ASSERT_EQUAL(42, mVehicleState.data.vehicle.gps.utcTime.millisec);
    TEST_ASSERT_EQUAL_STRING(expectedLat, mVehicleState.data.vehicle.gps.latitude);
    TEST_ASSERT_EQUAL_CHAR('N', mVehicleState.data.vehicle.gps.nsIndicator);
    TEST_ASSERT_EQUAL_STRING(expectedLong, mVehicleState.data.vehicle.gps.longitude);
    TEST_ASSERT_EQUAL_CHAR('E', mVehicleState.data.vehicle.gps.ewIndicator);
    TEST_ASSERT_EQUAL(10, mVehicleState.data.vehicle.gps.nSatellites);
}

TEST_GROUP_RUNNER(DEVICE_GPS)
{
    RUN_TEST_CASE(DEVICE_GPS, InitOk);
    RUN_TEST_CASE(DEVICE_GPS, GpsReceiveGPGGA);
}

#define INVOKE_TEST DEVICE_GPS
#include "test_main.h"
