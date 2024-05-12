/*
 * TestNmeaDecode.c
 * 
 *  Created on: 25 Oct 2022
 *      Author: Liam Flaherty
 */

#include "unity.h"
#include "unity_fixture.h"

#include <string.h>

// Mocks for code under test
#include "stm32_hal/MockStm32f7xx_hal.h"
#include "lib/logging/MockLogging.h"

// source code under test
#include "comm/uart/nmeadecode.c"

static Logging_T testLog;

static NmeaDecode_T mDecoder;

TEST_GROUP(COMM_NMEADECODE);

TEST_SETUP(COMM_NMEADECODE)
{
    // set up supporting modules
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_Init(&testLog));
    TEST_ASSERT_EQUAL(LOGGING_STATUS_OK, Log_EnableSWO(&testLog));
    mockLogClear();

    // set up NMEA decoder
    NmeaDecode_Init(&mDecoder);

    TEST_ASSERT_EQUAL(0U, mDecoder.start);
    TEST_ASSERT_EQUAL(0U, mDecoder.end);
    TEST_ASSERT_EQUAL(NMEAMSG_BUFFER_LEN, mDecoder.availableBytes);
}

TEST_TEAR_DOWN(COMM_NMEADECODE)
{
    // Empty
}

TEST(COMM_NMEADECODE, TestSimpleMsgGGA)
{
    const char expectedLat[] = "2236.2791";
    const char expectedLong[] = "12017.2818";
    const char msg[] = "$GPGGA,091626.042,2236.2791,N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*60\r\n";
    const uint16_t msgLen = (uint16_t)strlen(msg);

    bool succ = NmeaDecode_AccumulateBytes(&mDecoder, (uint8_t*)msg, msgLen);
    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(0U, mDecoder.start);
    TEST_ASSERT_EQUAL(msgLen, mDecoder.end);
    TEST_ASSERT_EQUAL(NMEAMSG_BUFFER_LEN - msgLen, mDecoder.availableBytes);

    NmeaMessageFields_T decodeFields;
    succ = NmeaDecode_Decode(&mDecoder, &decodeFields);
    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(NMEA_MSGTYPE_GPGGA, decodeFields.type);
    TEST_ASSERT_EQUAL(9, decodeFields.fields.gpgga.utcTime.hour);
    TEST_ASSERT_EQUAL(16, decodeFields.fields.gpgga.utcTime.min);
    TEST_ASSERT_EQUAL(26, decodeFields.fields.gpgga.utcTime.sec);
    TEST_ASSERT_EQUAL(42, decodeFields.fields.gpgga.utcTime.millisec);
    TEST_ASSERT_EQUAL_STRING(expectedLat, decodeFields.fields.gpgga.latitude);
    TEST_ASSERT_EQUAL_CHAR('N', decodeFields.fields.gpgga.nsIndicator);
    TEST_ASSERT_EQUAL_STRING(expectedLong, decodeFields.fields.gpgga.longitude);
    TEST_ASSERT_EQUAL_CHAR('E', decodeFields.fields.gpgga.ewIndicator);
    TEST_ASSERT_EQUAL(10, decodeFields.fields.gpgga.nSatellites);
}

TEST(COMM_NMEADECODE, TestBadChecksum)
{
    const char msg[] = "$GPGGA,091626.042,2236.2791,N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*Ff\r\n";
    const uint16_t msgLen = (uint16_t)strlen(msg);

    bool succ = NmeaDecode_AccumulateBytes(&mDecoder, (uint8_t*)msg, msgLen);
    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(0U, mDecoder.start);
    TEST_ASSERT_EQUAL(msgLen, mDecoder.end);
    TEST_ASSERT_EQUAL(NMEAMSG_BUFFER_LEN - msgLen, mDecoder.availableBytes);

    NmeaMessageFields_T decodeFields;
    succ = NmeaDecode_Decode(&mDecoder, &decodeFields);
    TEST_ASSERT_FALSE(succ);
    TEST_ASSERT_EQUAL(NMEA_MSGTYPE_NULL, decodeFields.type);
}

TEST(COMM_NMEADECODE, TestInvalidMsgType)
{
    const char msg[] = "$GPIDK,091626.042,2236.2791,N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*60\r\n";
    const uint16_t msgLen = (uint16_t)strlen(msg);

    bool succ = NmeaDecode_AccumulateBytes(&mDecoder, (uint8_t*)msg, msgLen);
    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(0U, mDecoder.start);
    TEST_ASSERT_EQUAL(msgLen, mDecoder.end);
    TEST_ASSERT_EQUAL(NMEAMSG_BUFFER_LEN - msgLen, mDecoder.availableBytes);

    NmeaMessageFields_T decodeFields;
    succ = NmeaDecode_Decode(&mDecoder, &decodeFields);
    TEST_ASSERT_FALSE(succ);
    TEST_ASSERT_EQUAL(NMEA_MSGTYPE_NULL, decodeFields.type);
}

TEST(COMM_NMEADECODE, TestTooManyBytesGGA)
{
    const char expectedLat[] = "2236.2791";
    const char expectedLong[] = "12017.2818";
    const char msg[] = "$GPGGA,$GPGGA,091626.042,2236.2791,N,12017.2818,E,1,10,1.00,8.8,M,18.7,M,,*60\r\nsomething,else\r\n";
    const uint16_t msgLen = (uint16_t)strlen(msg);

    bool succ = NmeaDecode_AccumulateBytes(&mDecoder, (uint8_t*)msg, msgLen);
    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(0U, mDecoder.start);
    TEST_ASSERT_EQUAL(msgLen, mDecoder.end);
    TEST_ASSERT_EQUAL(NMEAMSG_BUFFER_LEN - msgLen, mDecoder.availableBytes);

    NmeaMessageFields_T decodeFields;
    succ = NmeaDecode_Decode(&mDecoder, &decodeFields);
    TEST_ASSERT_TRUE(succ);
    TEST_ASSERT_EQUAL(NMEA_MSGTYPE_GPGGA, decodeFields.type);
    TEST_ASSERT_EQUAL(9, decodeFields.fields.gpgga.utcTime.hour);
    TEST_ASSERT_EQUAL(16, decodeFields.fields.gpgga.utcTime.min);
    TEST_ASSERT_EQUAL(26, decodeFields.fields.gpgga.utcTime.sec);
    TEST_ASSERT_EQUAL(42, decodeFields.fields.gpgga.utcTime.millisec);
    TEST_ASSERT_EQUAL_STRING(expectedLat, decodeFields.fields.gpgga.latitude);
    TEST_ASSERT_EQUAL_CHAR('N', decodeFields.fields.gpgga.nsIndicator);
    TEST_ASSERT_EQUAL_STRING(expectedLong, decodeFields.fields.gpgga.longitude);
    TEST_ASSERT_EQUAL_CHAR('E', decodeFields.fields.gpgga.ewIndicator);
    TEST_ASSERT_EQUAL(10, decodeFields.fields.gpgga.nSatellites);
}

TEST_GROUP_RUNNER(COMM_NMEADECODE)
{
    RUN_TEST_CASE(COMM_NMEADECODE, TestSimpleMsgGGA);
    RUN_TEST_CASE(COMM_NMEADECODE, TestBadChecksum);
    RUN_TEST_CASE(COMM_NMEADECODE, TestInvalidMsgType);
    RUN_TEST_CASE(COMM_NMEADECODE, TestTooManyBytesGGA);
}

#define INVOKE_TEST COMM_NMEADECODE
#include "test_main.h"
