/*
 * TestCan.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include "MockLogging.h"
#include "stm32_hal/MockStm32f7xx_hal.h"

#include "lib/logging/logging.h"
#include "comm/can/can.h"
 
static Logging_T mLog;

TEST_GROUP(COMM_CAN);

TEST_SETUP(COMM_CAN)
{
	mLog.enableLogToDebug = true;
	mLog.enableLogToLogFile = false;
	mLog.enableLogToSerial = false;
	mockLogClear();
}

TEST_TEAR_DOWN(COMM_CAN)
{
	mockLogClear();
}

TEST(COMM_CAN, TestName)
{
	
}

