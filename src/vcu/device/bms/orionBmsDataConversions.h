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
  return value / 10.0f;
}

/**
 * @returns degrees
 */
static inline float msgToAngle(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 10.0f;
}

/**
 * @returns rpm
 */
static inline int16_t msgToAngularVelocity(const uint16_t encodedInt)
{
  // It's just a re-cast to signed
  return (int16_t)encodedInt;
}

/**
 * @returns Hz
 */
static inline float msgToFrequency(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 10.0f;
}

/**
 * @returns amps (signed)
 */
static inline float msgToCurrent(const int16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 10.0f;
}

/**
 * @returns volts
 */
static inline float msgToVoltageHigh(const int16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 10.0f;
}

/**
 * @returns volts
 */
static inline float msgToVoltageLow(const int16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 100.0f;
}

/**
 * @returns Webers (Wb)
 */
static inline float msgToFlux(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 1000.0f;
}

/**
 * @returns Nm
 */
static inline float msgToTorque(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 10.0f;
}

/**
 * @return Torque in message integer format
 */
static inline int16_t torqueToMsg(const float torque)
{
  return (int16_t)(10.0f * torque);
}

/**
 * @returns Modulation index (unitless)
 */
static inline float msgToModulationIndex(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 100.0f;
}

/**
 * @returns Percent
 */
static inline float msgToPercent(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 100.0f;
}



#endif /* DEVICE_ORIONBMS_DATACONVERSIONS_H_ */
