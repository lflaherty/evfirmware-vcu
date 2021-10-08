/*
 * dataConversions.h
 *
 *  Created on: Oct 7 2021
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_INVERTER_DATACONVERSIONS_H_
#define DEVICE_INVERTER_DATACONVERSIONS_H_

/**
 * @returns degrees celsius
 */
static inline float msgToTemperature(const uint16_t encodedInt)
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
 * @returns amps
 */
static inline float msgToCurrent(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 10.0f;
}

/**
 * @returns volts
 */
static inline float msgToVoltageHigh(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 10.0f;
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
 * @returns Modulation index (unitless)
 */
static inline float msgToModulationIndex(const uint16_t encodedInt)
{
  float value = (float)encodedInt;
  return value / 100.0f;
}



#endif /* DEVICE_INVERTER_DATACONVERSIONS_H_ */
