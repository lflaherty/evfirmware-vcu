/*
 * TestCan.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include "MockLogging.h"
#include "lib/logging/logging.h"

// #include "comm/can/can.h"

TEST_GROUP(COMM_CAN);

TEST_SETUP(COMM_CAN)
{
}

TEST_TEAR_DOWN(COMM_CAN)
{
}

TEST(COMM_CAN, TestName)
{
	Logging_T logData;
	Logging_Status_T s = logPrintS(&logData, "teststr", 100);

	TEST_ASSERT(s == LOGGING_STATUS_OK);
	TEST_ASSERT_EQUAL_STRING("teststr", mockLogBuffer);
}

