/*
 * torqueMap.h
 *
 *  Created on: Jun 22 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLELOGIC_THROTTLECONTROLLER_TORQUEMAP_H_
#define VEHICLELOGIC_THROTTLECONTROLLER_TORQUEMAP_H_

#include <stdint.h>

#define TORQUEMAP_LENGTH ((uint16_t)5)

typedef struct
{
  float accel[TORQUEMAP_LENGTH]; // accelerator pedal %
  float torque[TORQUEMAP_LENGTH]; // motor torque
} TorqueMap_T;

extern const TorqueMap_T TorqueMap_Default;
extern const TorqueMap_T TorqueMap_DefaultReverse;

/**
 * @brief Linearly interpolate the given accelerator input on the torque map.
 * 
 * @param map Torque map
 * @param accel Sensor input
 * @return Interpolated result.
 */
float TorqueMap_Interpolate(const TorqueMap_T* map, float accel);

#endif // VEHICLELOGIC_THROTTLECONTROLLER_TORQUEMAP_H_
