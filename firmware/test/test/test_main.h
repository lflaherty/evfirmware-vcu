/*
 * run_tests.h
 *
 *  Created on: 24 Apr 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#ifdef INVOKE_TEST
#define RUN_MACRO(group) RUN_TEST_GROUP(group)
#else
#error "Define INVOKE_TEST to run test"
#endif

static void RunAllTests(void)
{
    RUN_MACRO(INVOKE_TEST);
}

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, RunAllTests);
}
