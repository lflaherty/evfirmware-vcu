/*
 * all_tests.c
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#include "unity_fixture.h"

static void RunAllTests(void)
{
	RUN_TEST_GROUP(COMM_CAN);
}

int main(int argc, const char* argv[])
{
	return UnityMain(argc, argv, RunAllTests);
}
