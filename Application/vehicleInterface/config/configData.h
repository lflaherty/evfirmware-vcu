/*
 * configData.h
 *
 *  Created on: Jun 8 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_CONFIG_CONFIGDATA_H_
#define VEHICLEINTERFACE_CONFIG_CONFIGDATA_H_

#include <stdint.h>

typedef struct
{
    uint16_t rawLower;
    uint16_t rawUpper;
} Config_PedalCalibration_T;

typedef struct
{
    Config_PedalCalibration_T calibrationA;
    Config_PedalCalibration_T calibrationB;
    float consistencyLimit; // % that both sensors can disagree by
    uint16_t invalidDataTimeout; // [ms]
} Config_Pedal_T;

typedef struct
{
    Config_Pedal_T accelPedal;
    Config_Pedal_T brakePressure;
    uint8_t pedalAbuseCheck; // 0=disabled, 1=enabled
} Config_InputSensors_T;

typedef struct
{
    uint32_t baseCanId;
    // Fault configuration:
    uint16_t maxCellTemp; // [deg Celsius]
    uint32_t maxCurrent; // max current draw [mA]
    uint16_t maxCellVoltage; // max voltage for any cell [mV]
    uint16_t invalidDataTimeout; // [ms]
    uint16_t canTimeout; // [ms]
} Config_BMS_T;

typedef struct
{
    uint32_t baseCanId;
    // Fault configuration:
    uint16_t maxInternalTemp; // [deg C]
    uint16_t maxIGBTTemp; // [deg C]
    uint16_t maxMotorTemp; // [deg C]
    uint32_t maxCurrentDraw; // [mA]
    uint16_t invalidDataTimeout; // [ms]
    uint16_t canTimeout; // [ms]
} Config_Inverter_T;

typedef struct
{
    Config_InputSensors_T inputs;
    Config_BMS_T bms;
    Config_Inverter_T inverter;
} Config_T;

#endif /* VEHICLEINTERFACE_CONFIG_CONFIGDATA_H_ */