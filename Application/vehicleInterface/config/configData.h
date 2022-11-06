/*
 * configData.h
 *
 *  Created on: Jun 8 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_CONFIG_CONFIGDATA_H_
#define VEHICLEINTERFACE_CONFIG_CONFIGDATA_H_

#include <stdint.h>

#include "vehicleInterface/vehicleState/vehicleStateTypes.h"

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
} Config_Pedal_T;

typedef struct
{
    Config_Pedal_T accelPedal;
    Config_Pedal_T brakePressure;
    uint8_t pedalAbuseCheckEnabled; // 0=disabled, 1=enabled
    float pedalAbuseAccelThreshold; // percent
    float pedalAbuseBrakeThreshold; // percent
    uint16_t invalidDataTimeout; // [ms]
} Config_InputSensors_T;

typedef struct
{
    uint32_t baseCanId;
    // Fault configuration:
    Temperature_T maxCellTemp;
    Current_T maxCurrent; // max current draw for battery pack
    LowVoltage_T maxCellVoltage; // max voltage for any cell
    Percent_T minStateOfCharge; // min state of charge for the battery pack
    uint16_t invalidDataTimeout; // [ms]
    uint16_t canTimeout; // [ms]
} Config_BMS_T;

typedef struct
{
    uint32_t baseCanId;
    // Fault configuration:
    Temperature_T maxInternalTemp;
    Temperature_T maxIGBTTemp;
    Temperature_T maxMotorTemp;
    Current_T maxCurrentDraw;
    uint16_t invalidDataTimeout; // [ms]
    uint16_t canTimeout; // [ms]
} Config_Inverter_T;

typedef struct
{
    uint32_t hvActiveStateWait; // ms to wait in HV active state
    uint32_t hvChargeTimeout; // Time which causes a HV charge timeout (ms)
} Config_VCULogic_T;

typedef struct
{
    Config_InputSensors_T inputs;
    Config_BMS_T bms;
    Config_Inverter_T inverter;
    Config_VCULogic_T vcu;
} Config_T;

#endif /* VEHICLEINTERFACE_CONFIG_CONFIGDATA_H_ */
