/*
 * vehicleStateTypes.h
 *
 *  Created on: Oct 8, 2021
 *      Author: Liam Flaherty
 */

#ifndef VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATETYPES_H_
#define VEHICLEINTERFACE_VEHICLESTATE_VEHICLESTATETYPES_H_

/**
 * Input sensors
  APPS 1/2/average
  BPS 1/2/average
  wheel speed front/rear

Battery data
  cell voltages
  cell temperatures
  idk what more.. todo

Motor data
  Temperature
  motor angle
  motor speed
  phase currents

Inverter data
  Temperatures
  output frequency
  DC bus current
  DC bus voltage
  output line-neural voltage
  flux/id-iq currents
  states
  faults
  flux weakening
 */

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
} VehicleState_Motor_T;

typedef struct
{
  float moduleATemperature; // Degrees C
  float moduleBTemperature; // Degrees C
  float moduleCTemperature; // Degrees C
  float gateDriverTemp; // Degrees C
  float controlBoardTemp; // Degrees C
  // TODO the reset
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
