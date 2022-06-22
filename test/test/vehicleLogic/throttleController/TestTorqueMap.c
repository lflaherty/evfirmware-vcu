/**
 * TestTorqueMap.c
 * 
 *  Created on: Jun 22 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>
#include <stdio.h>

// Mocks for code under test (replaces stubs)

// source code under test
#include "vehicleLogic/throttleController/torqueMap.c"

TEST_GROUP(VEHICLELOGIC_TORQUEMAP);

TEST_SETUP(VEHICLELOGIC_TORQUEMAP)
{
}

TEST_TEAR_DOWN(VEHICLELOGIC_TORQUEMAP)
{
}

TEST(VEHICLELOGIC_TORQUEMAP, Interpolate)
{
    float inputs[] =   {0.0f, 0.05f, 0.1f,  0.2f,  0.4f,   0.5f,   0.6f,   0.7f,   0.8f,   0.9f,   1.0f};
    float expected[] = {0.0f, 0.0f,  0.0f, 25.0f, 75.0f, 100.0f, 150.0f, 200.0f, 300.0f, 400.0f, 500.0f};
    size_t testLen = sizeof(inputs) / sizeof(float);

    for (size_t i = 0; i < testLen; ++i) {
        float torque = TorqueMap_Interpolate(&TorqueMap_Default, inputs[i]);
        TEST_ASSERT_EQUAL_FLOAT(expected[i], torque);
    }
}

TEST(VEHICLELOGIC_TORQUEMAP, OutOfRange)
{
    float torque;

    torque = TorqueMap_Interpolate(&TorqueMap_Default, -0.1f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, torque);

    torque = TorqueMap_Interpolate(&TorqueMap_Default, 1.1f);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, torque);
}

TEST_GROUP_RUNNER(VEHICLELOGIC_TORQUEMAP)
{
    RUN_TEST_CASE(VEHICLELOGIC_TORQUEMAP, Interpolate);
    RUN_TEST_CASE(VEHICLELOGIC_TORQUEMAP, OutOfRange);
}
