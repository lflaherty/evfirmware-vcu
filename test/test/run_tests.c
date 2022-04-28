/*
 * run_tests.c
 *
 *  Created on: 24 Apr 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"


static void RunAllTests(void)
{
    RUN_TEST_GROUP(COMM_CAN);
    RUN_TEST_GROUP(COMM_UART);
    RUN_TEST_GROUP(IO_ADC);
}

int main(int argc, const char* argv[])
{
    return UnityMain(argc, argv, RunAllTests);
}
