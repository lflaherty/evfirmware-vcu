/*
 * vcu_tests.c
 *
 *  Created on: 1 May 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

static void RunVCUTests(void)
{
    RUN_TEST_GROUP(VEHICLEINTERFACE_VEHICLESTATE);
    RUN_TEST_GROUP(VEHICLEINTERFACE_VEHICLECONTROL);
    RUN_TEST_GROUP(VEHICLELOGIC_VEHICLESTATEMANAGER);
    RUN_TEST_GROUP(VEHICLELOGIC_STATEMACHINE);
    RUN_TEST_GROUP(VEHICLELOGIC_FAULTMANAGER);
    RUN_TEST_GROUP(VEHICLELOGIC_THROTTLECONTROLLER);
    RUN_TEST_GROUP(VEHICLELOGIC_TORQUEMAP);
    RUN_TEST_GROUP(VEHICLELOGIC_WATCHDOGTRIGGER);
    RUN_TEST_GROUP(DEVICE_PCDEBUG);
}
