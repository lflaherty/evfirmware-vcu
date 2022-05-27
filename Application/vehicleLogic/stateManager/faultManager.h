/*
 * fault.h
 *
 *  Created on: May 14 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLELOGIC_STATEMANAGER_FAULT_H_
#define VEHICLELOGIC_STATEMANAGER_FAULT_H_

#include "lib/logging/logging.h"

typedef struct
{
  // Input data
  // TODO pointer to vehicle state

  // Internal storage
  
} FaultManager_T;

typedef enum
{
  FAULT_INIT      = 0x00U,
  FAULT_NO_FAULT  = 0x01U,
  FAULT_LV_ERROR  = 0x02U,
  FAULT_FAULT     = 0x03U
} FaultStatus_T;

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