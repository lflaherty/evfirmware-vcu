/*
 * orionBmsDataConversions.h
 *
 *  Created on: 15 Apr 2023
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_ORIONBMS_DATACONVERSIONS_H_
#define DEVICE_ORIONBMS_DATACONVERSIONS_H_

#include <stdint.h>

/**
 * @returns degrees celsius
 */
static inline float msgToTemperature(const int16_t encodedInt)
{
  float value = (float)encodedInt;
  return value;
}

/**
 * @returns amps (signed)
 */
static inline float msgToCurrent(const int16_t encodedInt)
{
  float value = (float)encodedInt;
  return value * 0.1f;
}

/**
 * @returns volts
 */
static inline float msgToVoltageHigh(const int16_t encodedInt)
{
  float value = (float)encodedInt;
  return value * 0.1f;
}

/**
 * @returns volts
 */
static inline float msgToVoltageLow(const int16_t encodedInt)
{
  float value = (float)encodedInt;
  return value * 0.0001f;
}

/**
 * @returns Percent
 */
static inline float msgToStateOfChargePct(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value * 0.5f;
}



#endif /* DEVICE_ORIONBMS_DATACONVERSIONS_H_ */
