/*
 * fault.h
 *
 *  Created on: May 14 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLELOGIC_STATEMANAGER_FAULT_H_
#define VEHICLELOGIC_STATEMANAGER_FAULT_H_

#include <stdint.h>

#include "logging/logging.h"
#include "vehicleInterface/vehicleState/vehicleState.h"
#include "vehicleInterface/config/configData.h"

typedef struct
{
  uint32_t faults; // used to latch fault bits

  uint16_t accelPedalRangeTimer;
  uint16_t accelPedalRangeTimerLimit;
  uint16_t accelPedalConsistencyTimer;
  uint16_t accelPedalConsistencyTimerLimit;
  uint16_t brakePressureRangeTimer;
  uint16_t brakePressureRangeTimerLimit;
  uint16_t brakePressureConsistencyTimer;
  uint16_t brakePressureConsistencyTimerLimit;
  uint16_t pedalAbuseTimer;
  uint16_t pedalAbuseTimerLimit;
  uint16_t cellTempOverTimer;
  uint16_t currentOverDrawTimer;
  uint16_t cellVoltageOverTimer;
  uint16_t bmsFaultTimerLimit;
} FaultManager_Internal_T;

typedef struct
{
  // Config
  uint32_t tickRateMs; // milliseconds between each step
  Config_T* vehicleConfig;

  // Input data
  VehicleState_T* vehicleData; // must be set prior to initialization

  // Internal storage
  // Local variables - use a separate struct for easier memset operations
  FaultManager_Internal_T internal;
} FaultManager_T;

typedef enum
{
  FAULT_INIT      = 0x00U,
  FAULT_NO_FAULT  = 0x01U,
  FAULT_LV_ERROR  = 0x02U,
  FAULT_FAULT     = 0x03U
} FaultStatus_T;

// Fault categories
#define FAULTMGR_LV_ERROR_MASK  ((uint32_t)0x000000FFU) /* LV error */
#define FAULTMGR_FAULT_MASK     ((uint32_t)0xFFFFFF00U) /* Fault */

#define FAULTMGR_NO_FAULT             ((uint32_t)0)

// LV errors
#define FAULTMGR_LV_ERROR_BMS_TIMEOUT ((uint32_t)0x00000001U)   /* BMS CAN message timeout */
#define FAULTMGR_LV_ERROR_INV_TIMEOUT ((uint32_t)0x00000002U)   /* Inverter CAN message timeout */
#define FAULTMGR_LV_ERROR_INV_STATE   ((uint32_t)0x00000002U)   /* Inverter LV error state */

// Faults
#define FAULTMGR_FAULT_ACCELPDL_RANGE ((uint32_t)0x00000100U)   /* Accelerator: pedal outside calibrated range */
#define FAULTMGR_FAULT_ACCELPDL_SENSE ((uint32_t)0x00000200U)   /* Accelerator: pedal redunant sensors disagree */
#define FAULTMGR_FAULT_BRAKEPDL_RANGE ((uint32_t)0x00000400U)   /* Brake: pedal outside calibrated range */
#define FAULTMGR_FAULT_BRAKEPDL_SENSE ((uint32_t)0x00000800U)   /* Brake: pedal redunant sensors disagree */
#define FAULTMGR_FAULT_BRAKEPDL_ABUSE ((uint32_t)0x00001000U)   /* Brake: Accelerator & brake pedal depressed */
#define FAULTMGR_FAULT_BMS_CELLTEMP   ((uint32_t)0x00002000U)   /* BMS: A cell is above temperature threshold */
#define FAULTMGR_FAULT_BMS_CURRENT    ((uint32_t)0x00004000U)   /* BMS: Current draw too high */
#define FAULTMGR_FAULT_BMS_CELLVOLT   ((uint32_t)0x00008000U)   /* BMS: A cell is over voltage */
#define FAULTMGR_FAULT_BMS_LOWSOC     ((uint32_t)0x00010000U)   /* BMS: Total state of charge is below threshold */
#define FAULTMGR_FAULT_BMS_FAULTIND   ((uint32_t)0x00020000U)   /* BMS: Fault indicator is on */
#define FAULTMGR_FAULT_INV_OVERTEMP   ((uint32_t)0x00040000U)   /* Inverter: Internal temperature is above threshold */
#define FAULTMGR_FAULT_INV_IGBTTEMP   ((uint32_t)0x00080000U)   /* Inverter: IGBT temperature is above threshold */
#define FAULTMGR_FAULT_INV_MOTORTEMP  ((uint32_t)0x00100000U)   /* Inverter: Motor temperature is above threshold */
#define FAULTMGR_FAULT_INV_CURRENT    ((uint32_t)0x00200000U)   /* Inverter: Current draw too high */
#define FAULTMGR_FAULT_INV_FAULTIND   ((uint32_t)0x00400000U)   /* Inverter: Fault indicator (discrete input) */
#define FAULTMGR_FAULT_INV_FAULTMSG   ((uint32_t)0x00800000U)   /* Inverter: Fault message */
#define FAULTMGR_FAULT_INV_STATE      ((uint32_t)0x01000000U)   /* Inverter: Fault state */

/**
 * @brief Initialize the fault manager
 * 
 * @param logger Pointer to system logger
 * @param faultMgr Pointer to FaultManager struct
 */
void FaultManager_Init(Logging_T* logger, FaultManager_T* faultMgr);

/**
 * @brief Determines whether a fault has occured
 * 
 * @param faultMgr Pointer to FaultManager struct
 * @return FAULT_NO_FAULT if everything ok.
 * @return FAULT_LV_ERROR for LV specific errors
 * @return FAULT_FAULT if a fault is detected (HV or LV+HV)
 */
FaultStatus_T FaultManager_Step(FaultManager_T* faultMgr);

#endif // VEHICLELOGIC_STATEMANAGER_FAULT_H_
