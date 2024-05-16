/*
 * torqueMap.c
 *
 *  Created on: Jun 22 2022
 *      Author: Liam Flaherty
 */

#include "torqueMap.h"

const TorqueMap_T TorqueMap_Default =
{
  .accel =  {0.0f, 0.1f,   0.5f,   0.7f,   1.0f},
  .torque = {0.0f, 0.0f, 100.0f, 200.0f, 500.0f}
};
const TorqueMap_T TorqueMap_DefaultReverse =
{
  .accel =  {0.0f, 0.1f,   0.5f,   0.7f,   1.0f},
  .torque = {0.0f, 0.0f,  20.0f,  75.0f,  75.0f}
};

float TorqueMap_Interpolate(const TorqueMap_T* map, float accel)
{
  uint16_t idxLower = 0U;
  uint16_t idxUpper = 0U;

  for (uint16_t i = 0; i < (TORQUEMAP_LENGTH - 1); ++i) {
    if (accel >= map->accel[i] && accel <= map->accel[i+1]) {
      // found bucket
      idxLower = i;
      idxUpper = i+1;
    }
  }

  if (idxLower == idxUpper) {
    return 0.0f;
  }

  // do interpolation
  float accel0 = map->accel[idxLower];
  float accel1 = map->accel[idxUpper];
  float torque0 = map->torque[idxLower];
  float torque1 = map->torque[idxUpper];
  
  float t = (accel - accel0) / (accel1 - accel0);
  return (1 - t) * torque0 + t * torque1;
}
