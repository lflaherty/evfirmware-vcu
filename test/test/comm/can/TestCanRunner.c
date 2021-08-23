/*
 * TestCanRunner.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER(COMM_CAN)
{
	RUN_TEST_CASE(COMM_CAN, TestName);
}
