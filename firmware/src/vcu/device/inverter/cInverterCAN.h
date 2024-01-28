/*
 * cInverterCAN_IDs.h
 *
 *  Created on: Oct 7 2021
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_INVERTER_CINVERTERCAN_H_
#define DEVICE_INVERTER_CINVERTERCAN_H_

#define CINVERTER_CAN_ID_TEMPERATURES1       ((uint16_t) 0x0A0U)
#define CINVERTER_CAN_ID_TEMPERATURES2       ((uint16_t) 0x0A1U)
#define CINVERTER_CAN_ID_TEMPERATURES3       ((uint16_t) 0x0A2U)
#define CINVERTER_CAN_ID_MOTOR_POS_INFO      ((uint16_t) 0x0A5U)
#define CINVERTER_CAN_ID_CURRENT_INFO        ((uint16_t) 0x0A6U)
#define CINVERTER_CAN_ID_VOLTAGE_INFO        ((uint16_t) 0x0A7U)
#define CINVERTER_CAN_ID_FLUX_INFO           ((uint16_t) 0x0A8U)
#define CINVERTER_CAN_ID_INTERNAL_STATES     ((uint16_t) 0x0AAU)
#define CINVERTER_CAN_ID_FAULT_CODES         ((uint16_t) 0x0ABU)
#define CINVERTER_CAN_ID_TORQUE_TIMER        ((uint16_t) 0x0ACU)
#define CINVERTER_CAN_ID_FLUX_WEAKENING      ((uint16_t) 0x0ADU)

#define CINVERTER_CAN_ID_COMMAND             ((uint16_t) 0x0C0U)


typedef union
{
  struct {
    uint16_t moduleATemp;
    uint16_t moduleBTemp;
    uint16_t moduleCTemp;
    uint16_t gateDriverTemp;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_Temperatures1_T;

typedef union
{
  struct {
    uint16_t controlBoardTemp;
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t reserved3;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_Temperatures2_T;

typedef union
{
  struct {
    uint16_t reserved1;
    uint16_t reserved2;
    uint16_t motorTemp;
    uint16_t reserved3;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_Temperatures3_T;

typedef union
{
  struct {
    uint16_t motorAngle;
    uint16_t motorSpeed;
    uint16_t electricalOutFreq;
    uint16_t reserved1;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_MotorPosInfo_T;

typedef union
{
  struct {
    int16_t phaseACurrent;
    int16_t phaseBCurrent;
    int16_t phaseCCurrent;
    int16_t dcBusCurrent;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_CurrentInfo_T;

typedef union
{
  struct {
    uint16_t dcBusVoltage;
    uint16_t outputVoltage;
    uint16_t vd;
    uint16_t vq;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_VoltageInfo_T;

typedef union
{
  struct {
    uint16_t fluxCommand;
    uint16_t fluxFeedback;
    int16_t idFeedback;
    int16_t iqFeedback;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_FluxInfo_T;

typedef union
{
  struct {
    uint8_t vsmState;
    uint8_t reserved1;
    uint8_t inverterState;
    uint8_t relayState;
    uint8_t inverterRunMode : 1;
    uint8_t reserved2 : 4;
    uint8_t activeDischargeState : 3;
    uint8_t reserved3;
    uint8_t inverterEnabled : 1;
    uint8_t reversed4 : 7;
    uint8_t direction : 1;
    uint8_t reserved5 : 7;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_InternalStates_T;

typedef union
{
  struct {
    uint32_t postFault;
    uint32_t runFault;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_FaultCodes_T;

typedef union
{
  struct {
    uint16_t commandedTorque;
    uint16_t feedbackTorque;
    uint32_t timer;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_TorqueTimer_T;

typedef union
{
  struct {
    uint16_t modulationIndex;
    int16_t fluxWeakeningOutput;
    int16_t idCommand;
    int16_t iqCommand;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_FluxWeakening_T;

typedef union {
  struct {
    int16_t torqueNm;
    int16_t speed;
    uint8_t directionCommand;
    uint8_t inverterEnable : 1;
    uint8_t inverterDischarge : 1;
    uint8_t speedModeEnabled : 1;
    uint8_t unused : 5;
    int16_t torqueLim;
  } fields;
  uint8_t raw[8];
} CInverter_CAN_Command_T;


#endif /* DEVICE_INVERTER_CINVERTERCAN_H_ */
