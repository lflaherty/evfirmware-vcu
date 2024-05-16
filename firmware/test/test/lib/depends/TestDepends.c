/*
 * TestLogging.c
 *
 *  Created on: 3 Aug 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>

// source code under test
#include "depends/depends.c"

// Define some test components:

// Mock component -----------------------------------------
REGISTERED_MODULE_STATIC(LIB_COMPONENTA); // normally in header
REGISTERED_MODULE_STATIC_DEF(LIB_COMPONENTA);

typedef enum {
    LIBA_STATUS_OK = 0x00,
    LIBA_STATUS_ERROR = 0x01,
} LIBA_Status_T;

LIBA_Status_T LIBA_Init(void)
{
    REGISTER_STATIC(LIB_COMPONENTA, LIBA_STATUS_ERROR);
    return LIBA_STATUS_OK;
}

// Mock component -----------------------------------------
typedef struct {
    REGISTERED_MODULE();
} App1_T;

typedef enum {
    APP1_STATUS_OK = 0x42,
    APP1_STATUS_ERROR_DEPEND = 0x43,
    APP1_STATUS_ERROR_REGISTER = 0x44,
} App1_Status_T;

App1_Status_T App1_Init(App1_T* app1)
{
    DEPEND_ON_STATIC(LIB_COMPONENTA, APP1_STATUS_ERROR_DEPEND);
    REGISTER(app1, APP1_STATUS_ERROR_REGISTER);
    return APP1_STATUS_OK;
}

// Mock component -----------------------------------------
typedef struct {
    App1_T* app1;

    REGISTERED_MODULE();
} App2_T;

typedef enum {
    APP2_STATUS_OK = 0x00,
    APP2_STATUS_ERROR_DEPEND = 0x01,
    APP2_STATUS_ERROR_REGISTER = 0x02,
} App2_Status_T;

App2_Status_T App2_Init(App2_T* app2)
{
    DEPEND_ON(app2->app1, APP2_STATUS_ERROR_DEPEND);
    REGISTER(app2, APP2_STATUS_ERROR_REGISTER);
    return APP2_STATUS_OK;
}
// --------------------------------------------------------

TEST_GROUP(LIB_DEPENDS);

TEST_SETUP(LIB_DEPENDS)
{
    // Empty
}

TEST_TEAR_DOWN(LIB_DEPENDS)
{
    // Reset the depends
    nModules = 1U;
}

TEST(LIB_DEPENDS, TestOk)
{
    App1_T app1;
    App2_T app2;
    app2.app1 = &app1;

    TEST_ASSERT_EQUAL(LIBA_STATUS_OK, LIBA_Init());
    TEST_ASSERT_EQUAL(APP1_STATUS_OK, App1_Init(&app1));
    TEST_ASSERT_EQUAL(APP2_STATUS_OK, App2_Init(&app2));
}

TEST(LIB_DEPENDS, TestDependFail)
{
    App1_T app1;
    App2_T app2;
    app2.app1 = &app1;

    TEST_ASSERT_EQUAL(LIBA_STATUS_OK, LIBA_Init());
    TEST_ASSERT_EQUAL(APP2_STATUS_ERROR_DEPEND, App2_Init(&app2));
}

TEST(LIB_DEPENDS, TestNullDepend)
{
    App1_T app1 = {0};
    App2_T app2 = {0};
    // Not setting app2.app1 to leave it null

    TEST_ASSERT_EQUAL(LIBA_STATUS_OK, LIBA_Init());
    TEST_ASSERT_EQUAL(APP1_STATUS_OK, App1_Init(&app1));
    TEST_ASSERT_EQUAL(APP2_STATUS_ERROR_DEPEND, App2_Init(&app2));
}

TEST(LIB_DEPENDS, TestStaticDependFail)
{
    App1_T app1;
    TEST_ASSERT_EQUAL(APP1_STATUS_ERROR_DEPEND, App1_Init(&app1));
}

TEST_GROUP_RUNNER(LIB_DEPENDS)
{
    RUN_TEST_CASE(LIB_DEPENDS, TestOk);
    RUN_TEST_CASE(LIB_DEPENDS, TestDependFail);
    RUN_TEST_CASE(LIB_DEPENDS, TestNullDepend);
    RUN_TEST_CASE(LIB_DEPENDS, TestStaticDependFail);
}

#define INVOKE_TEST LIB_DEPENDS
#include "test_main.h"
