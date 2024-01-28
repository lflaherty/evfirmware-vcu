/*
 * run_tests.c
 *
 *  Created on: 24 Apr 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include "system_tests.c"
#include "vcu_tests.c"

static void RunAllTests(void)
{
    RunSystemTests();
    RunVCUTests();
}

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, RunAllTests);
}
