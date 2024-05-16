/*
 * configDataDefault.h
 *
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_CONFIG_CONFIGDATADEFAULT_H_
#define VEHICLEINTERFACE_CONFIG_CONFIGDATADEFAULT_H_

#include <stdint.h>

#include "vehicleInterface/vehicleState/vehicleStateTypes.h"
#include "configData.h"

static Config_T DefaultConfigData = {
  .inputs = {
    .accelPedal = {
      .calibrationA = {
        .rawLower = 0,
        .rawUpper = 4095,
      },
      .calibrationB = {
        .rawLower = 0,
        .rawUpper = 4095,
      },
      .consistencyLimit = 0.1f,
    },
    .brakePressureFront = {
      .rawLower = 0,
      .rawUpper = 4095,
    },
    .brakePressureRear = {
      .rawLower = 0,
      .rawUpper = 4095,
    },
    .pedalAbuseCheckEnabled = 0, // disabled
    .pedalAbuseAccelThreshold = 0.1f,
    .pedalAbuseBrakeThreshold = 0.1f,
    .invalidDataTimeout = 100, // ms
    .numWheelspeedTeeth = 12,
  },
  .bms = {
    .baseCanId = 0x300,
    .maxCellTemp = 750, // 75 degC
    .maxCurrent = 4500, // 450 Amps
    .maxCellVoltage = 420, // 4.2 V
    .minStateOfCharge = 1000, // 10%
    .invalidDataTimeout = 100, // 100ms,
    .canTimeout = 100, // 100ms
  },
  .inverter = {
    .baseCanId = 0x0,
    .maxInternalTemp = 750, // 75 degC
    .maxIGBTTemp = 750, // 75 degC
    .maxMotorTemp = 750, // 75 degC
    .maxCurrentDraw = 4500, // 450 Amps,
    .invalidDataTimeout = 100, // 100ms,
    .canTimeout = 100, // 100ms
  },
  .vcu = {
    .hvActiveStateWait = 500, // 500ms
    .hvChargeTimeout = 7000, // 7s
  },
};

#endif /* VEHICLEINTERFACE_CONFIG_CONFIGDATADEFAULT_H_ */
