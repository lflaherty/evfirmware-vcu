/*
 * vehicleStateTypes.h
 *
 *  Created on: Oct 8, 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATETYPES_H_
#define VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATETYPES_H_

#include <stdbool.h>
#include <stdint.h>

#include "uart/nmeatypes.h"

typedef int16_t Temperature_T;    // x10   -3276.8 to +3276.7 °C
typedef int16_t LowVoltage_T;     // x100  -327.68 to +327.67 volts
typedef int16_t HighVoltage_T;    // x10   -3276.8 to +3276.7 volts
typedef int16_t Torque_T;         // x10   -3276.8 to +3276.7 Nm
typedef int16_t Current_T;        // x10   -3276.8 to +3276.7 amps
typedef uint16_t Percent_T;       // x100     0.00 to +100.00 %
typedef uint16_t RPM_T;           // x1          0 to 65,535 RPM

typedef struct
{
  float accel; // accelerator pedal press % [0,1]
  float accelA;
  float accelB;
  uint16_t accelRawA; // accelerator sensor A press raw sensor value
  uint16_t accelRawB; // accelerator sensor B press raw sensor value
  bool accelValid;

  float brakePresFront;
  float brakePresRear;
  uint16_t brakeRawFront; // front brake sensor pressure raw sensor value
  uint16_t brakeRawRear; // rear front brake sensor pressure raw sensor value
} VehicleState_InputSensors_T;

typedef struct
{
  bool buttonPressed;
  bool ledOn;
} VehicleState_Dash_T;

typedef struct
{
  // True => error raised
  bool bms;
  bool bspd;
  bool imd;
  bool out; // overall output status for SDC
} VehicleState_SDC_T;

#define VEHICLESTATE_MAXPDM_CHANNELS 8U
typedef struct
{
  bool pdmChState[VEHICLESTATE_MAXPDM_CHANNELS];
} VehicleState_GLV_T; // Grounded LV system

typedef struct
{
  NmeaUTCTime_T utcTime;
  char latitude[11];
  char nsIndicator; // 'N'/'S' north/south
  char longitude[11];
  char ewIndicator; // 'E'/'W' east/west
  uint8_t positionFix; 
  uint8_t nSatellites;
} VehicleState_GPS_T;

typedef struct
{
  RPM_T wheelspeedFront;
  RPM_T wheelspeedRear;

  uint16_t wssCountFront;
  uint16_t wssCountRear;
} VehicleState_Wheelspeed_T;

typedef struct
{
  VehicleState_GPS_T gps;
  VehicleState_SDC_T sdc;
  VehicleState_Wheelspeed_T wheelspeed;

  // TODO IMU dynamics
} VehicleState_VehicleSensors_T;

typedef struct
{
  float maxCellVoltage;             // highest voltage of any cell
  float maxCellTemperature;         // highest temp. of any cell
  float minCellVoltage;             // lowest voltage of any cell
  float minCellTemperature;         // lowest temp. of any cell
  uint8_t maxCellVoltageCellID;     // Cell with highest voltage
  uint8_t maxCellTemperatureCellID; // Cell with highest temp
  uint8_t minCellVoltageCellID;     // Cell with lowest voltage
  uint8_t minCellTemperatureCellID; // Cell with lowest temp
  float dcCurrent;                  // Current draw from battery
  float dcVoltage;                  // Total battery pack voltage
  float stateOfCarge;               // Battery pack SoC
  bool bmsFaultIndicator;           // BMS indicates fault
  uint8_t bmsPopulatedCells;        // Number of cells connected to BMS
  uint8_t bmsCounter;               // Increments every BMS counter message
  uint16_t bmsFailsafeStatus;       // Current failsafe mode status
} VehicleState_Battery_T;

typedef struct
{
  float temperature; // Degrees C
  float angle; // degrees
  int16_t speed; // rpm
  float phaseACurrent; // amps
  float phaseBCurrent; // amps
  float phaseCCurrent; // amps
  float calculatedTorque; // Nm
} VehicleState_Motor_T;

typedef enum
{
  VEHICLESTATE_INVERTERVSMSTATE_START = 0x0U,
  VEHICLESTATE_INVERTERVSMSTATE_PRECHARGEINIT = 0x1U,
  VEHICLESTATE_INVERTERVSMSTATE_PRECHARGEACTIVE = 0x2U,
  VEHICLESTATE_INVERTERVSMSTATE_PRECHARGECOMPLETE = 0x3U,
  VEHICLESTATE_INVERTERVSMSTATE_WAIT = 0x4U,
  VEHICLESTATE_INVERTERVSMSTATE_READY = 0x5U,
  VEHICLESTATE_INVERTERVSMSTATE_MOTORRUNNING = 0x6U,
  VEHICLESTATE_INVERTERVSMSTATE_FAULT = 0x7U
} VehicleState_InverterVSMState_T;

typedef enum
{
  VEHICLESTATE_INVERTERSTATE_POWERON = 0x0U,
  VEHICLESTATE_INVERTERSTATE_STOP = 0x1U,
  VEHICLESTATE_INVERTERSTATE_OPENLOOP = 0x2U,
  VEHICLESTATE_INVERTERSTATE_CLOSEDLOOP = 0x3U,
  VEHICLESTATE_INVERTERSTATE_IDLERUN = 0x8U,
  VEHICLESTATE_INVERTERSTATE_IDLESTOP = 0x9U
} VehicleState_InverterState_T;

typedef enum
{
  VEHICLESTATE_DISCHARGESTATE_DISABLED    = 0x0U,
  VEHICLESTATE_DISCHARGESTATE_ENABLEDWAIT = 0x1U,
  VEHICLESTATE_DISCHARGESTATE_SPEEDCHECK  = 0x2U,
  VEHICLESTATE_DISCHARGESTATE_ACTIVE      = 0x3U,
  VEHICLESTATE_DISCHARGESTATE_COMPLETE    = 0x4U
} VehicleState_InverterDischargeState_T;

typedef enum
{
  VEHICLESTATE_INVERTER_DISABLED = 0x0U,
  VEHICLESTATE_INVERTER_ENABLED  = 0x1U
} VehicleState_InverterEnabled_T;

typedef enum
{
  VEHICLESTATE_INVERTER_REVERSE  = 0x0U,
  VEHICLESTATE_INVERTER_FORWARD  = 0x1U
} VehicleState_InverterDirection_T;

typedef struct
{
  // physical data
  float moduleATemperature; // Degrees C
  float moduleBTemperature; // Degrees C
  float moduleCTemperature; // Degrees C
  float gateDriverTemp; // Degrees C
  float controlBoardTemp; // Degrees C
  float outputFrequency; // Hz
  float dcBusCurrent; // amps
  float dcBusVoltage; // amps
  float outputVoltage; // line-neural volts
  float vd; // D-axis voltage [V]
  float vq; // Q-axis voltage [Q]
  float fluxCommand; // Wb
  float fluxFeedback; // Wb
  float idFeedback; // Amps
  float iqFeedback; // Amps
  float idCommand; // Amps
  float iqCommand; // Amps
  float commandedTorque; // Nm
  float modulationIndex; // p.u.
  float fluxWeakeningOutput; // Amps
  // state data
  VehicleState_InverterVSMState_T vsmState;
  VehicleState_InverterState_T inverterState;
  VehicleState_InverterDischargeState_T dischargeState;
  VehicleState_InverterEnabled_T enabled;
  VehicleState_InverterDirection_T direction;
  uint32_t timerCounts; // 3ms counts
  uint32_t runFaults; // binary encoded
  uint32_t postFaults; // binary encoded
} VehicleState_Inverter_T;

typedef struct
{
  VehicleState_InputSensors_T inputs;
  VehicleState_Dash_T dash;
  VehicleState_VehicleSensors_T vehicle;
  VehicleState_GLV_T glv;
  VehicleState_Battery_T battery;
  VehicleState_Motor_T motor;
  VehicleState_Inverter_T inverter;
} VehicleState_Data_T;

#endif /* VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATETYPES_H_ */
