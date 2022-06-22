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
  float continuousTorque; // Nm to cap at for continuous torque mode
  uint32_t peakTorqueMs; // ms allowed at peak torque
  uint32_t resetTimeMs; // ms at continuous Tq to allow peak again
} TorqueMap_T;

static TorqueMap_T TorqueMap_Default =
{
  .accel =  {0.0f, 0.1f,   0.5f,   0.7f,   1.0f},
  .torque = {0.0f, 0.0f, 100.0f, 200.0f, 500.0f},
  .continuousTorque = 213.0f,
  .peakTorqueMs = 3000U,
  .resetTimeMs = 3000U
};
static TorqueMap_T TorqueMap_DefaultReverse =
{
  .accel =  {0.0f, 0.1f,   0.5f,   0.7f,   1.0f},
  .torque = {0.0f, 0.0f,  20.0f,  75.0f,  75.0f},
  .continuousTorque = 75.0f,
  .peakTorqueMs = 3000U,
  .resetTimeMs = 3000U
};

/**
 * @brief Linearly interpolate the given accelerator input on the torque map.
 * 
 * @param map Torque map
 * @param accel Sensor input
 * @return Interpolated result.
 */
float TorqueMap_Interpolate(TorqueMap_T* map, float accel);

#endif // VEHICLELOGIC_THROTTLECONTROLLER_TORQUEMAP_H_