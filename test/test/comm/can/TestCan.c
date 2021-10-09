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

TEST_GROUP(COMM_CAN);

TEST_SETUP(COMM_CAN)
{
	mockLogClear();
}

TEST_TEAR_DOWN(COMM_CAN)
{
	mockLogClear();
}

TEST(COMM_CAN, TestName)
{
	Logging_T logData;
	Logging_Status_T s = LOGGING_STATUS_OK;
	s |= logPrintS(&logData, "teststr\n", 100);
	s |= logPrintS(&logData, "teststr2\n", 100);

	mockLogDisplay();

	TEST_ASSERT(s == LOGGING_STATUS_OK);
	TEST_ASSERT_EQUAL_STRING("teststr\nteststr2\n", mockLogGet());
}

