/*
 * fault.h
 *
 *  Created on: May 14 2022
 *      Author: Liam Flaherty
 */

#include "faultManager.h"

// ------------------- Private data -------------------
static Logging_T* mLog;

// ------------------- Private methods -------------------
static uint32_t isFaultAccelPedal(VehicleState_Data_T* data)
{
  (void)data;
  return 0x0U;
}

static uint32_t isFaultBrakePedal(VehicleState_Data_T* data)
{
  (void)data;
  return 0x0U;
}

static uint32_t isFaultBMS(VehicleState_Data_T* data)
{
  (void)data;
  return 0x0U;
}

static uint32_t isFaultInverter(VehicleState_Data_T* data)
{
  (void)data;
  return 0x0U;
}

static uint32_t isLVErrorBMS(VehicleState_Data_T* data)
{
  (void)data;
  return 0x0U;
}

static uint32_t isLVErrorInverter(VehicleState_Data_T* data)
{
  (void)data;
  return 0x0U;
}

// ------------------- Public methods -------------------
void FaultManager_Init(Logging_T* logger, FaultManager_T* faultMgr)
{
  mLog = logger;
  logPrintS(mLog, "FaultManager_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  (void)faultMgr;

  logPrintS(mLog, "FaultManager_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
}

FaultStatus_T FaultManager_Step(FaultManager_T* faultMgr)
{
  // Acquire data
  VehicleState_Data_T latestData;
  bool stateAccess = VehicleState_CopyState(faultMgr->vehicleData, &latestData);
  if (!stateAccess) {
    return FAULT_LV_ERROR;
  }

  // Run checks on data
  uint32_t faultStatus = 0U;
  faultStatus |= isFaultAccelPedal(&latestData);
  faultStatus |= isFaultBrakePedal(&latestData);
  faultStatus |= isFaultBMS(&latestData);
  faultStatus |= isFaultInverter(&latestData);
  faultStatus |= isLVErrorBMS(&latestData);
  faultStatus |= isLVErrorInverter(&latestData);

  if ((faultStatus & FAULTMGR_FAULT_MASK) > 0U) {
    return FAULT_FAULT;
  } else if ((faultStatus & FAULTMGR_LV_ERROR_MASK) > 0U) {
    return FAULT_LV_ERROR;
  } else {
    return FAULT_NO_FAULT;
  }
}