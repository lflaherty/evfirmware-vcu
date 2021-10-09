/*
 * TestCan.c
 *
 *  Created on: 9 Oct 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include "MockLogging.h"

#include "lib/logging/logging.h"

static Logging_T mLog;

TEST_GROUP(LIB_LOGGING_MOCK);

TEST_SETUP(LIB_LOGGING_MOCK)
{
	mLog.enableLogToDebug = true;
	mLog.enableLogToLogFile = false;
	mLog.enableLogToSerial = false;
	mockLogClear();
}

TEST_TEAR_DOWN(LIB_LOGGING_MOCK)
{
	mockLogClear();
}

TEST(LIB_LOGGING_MOCK, BasicTest)
{
	Logging_Status_T s = LOGGING_STATUS_OK;
	s |= logPrintS(&mLog, "teststr1\n", 100);
	s |= logPrintS(&mLog, "teststr2\n", 100);

	TEST_ASSERT(s == LOGGING_STATUS_OK);
	TEST_ASSERT_EQUAL_STRING("teststr1\nteststr2\n", mockLogGet());
}

TEST_GROUP_RUNNER(LIB_LOGGING_MOCK)
{
    RUN_TEST_CASE(LIB_LOGGING_MOCK, BasicTest);
}