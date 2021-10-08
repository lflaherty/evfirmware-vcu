/*
 * vehicleStateTypes.h
 *
 *  Created on: Oct 8, 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATETYPES_H_
#define VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATETYPES_H_


typedef struct
{
  // TODO
  // throttle input
  // brake input
} VehicleState_InputSensors_T;

typedef struct
{
  // TODO
  // wheel speed
  // IMU dynamics
} VehicleState_VehicleSensors_T;

typedef struct
{
  // TODO
  // cell voltages/temperatures
} VehicleState_Battery_T;

typedef struct
{
  float temperature; // Degrees C
  float angle; // degrees
  float speed; // rpm
  float phaseACurrent; // amps
  float phaseBCurrent; // amps
  float phaseCCurrent; // amps
  float calculatedTorque; // Nm
} VehicleState_Motor_T;

typedef enum
{
  VEHICLESTATE_INVERTERSTATE_START = 0x0U,
  VEHICLESTATE_INVERTERSTATE_PRECHARGEINIT = 0x1U,
  VEHICLESTATE_INVERTERSTATE_PRECHARGEACTIVE = 0x2U,
  VEHICLESTATE_INVERTERSTATE_PRECHARGECOMPLETE = 0x3U,
  VEHICLESTATE_INVERTERSTATE_WAIT = 0x4U,
  VEHICLESTATE_INVERTERSTATE_READY = 0x5U,
  VEHICLESTATE_INVERTERSTATE_MOTORRUNNING = 0x6U,
  VEHICLESTATE_INVERTERSTATE_FAULT = 0x7U
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
  VehicleState_InverterState_T state;
  VehicleState_InverterDischargeState_T dischargeState;
  VehicleState_InverterEnabled_T enabled;
  VehicleState_InverterDirection_T direction;
  uint32_t timer; // milliseconds
  uint32_t runFaults; // binary encoded
  uint32_t postFaults; // binary encoded
} VehicleState_Inverter_T;

typedef struct
{
  VehicleState_InputSensors_T inputs;
  VehicleState_VehicleSensors_T vehicle;
  VehicleState_Battery_T battery;
  VehicleState_Motor_T motor;
  VehicleState_Inverter_T inverter;
} VehicleState_Data_T;

#endif /* VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATETYPES_H_ */
