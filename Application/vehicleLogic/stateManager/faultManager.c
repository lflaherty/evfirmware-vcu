/*
 * fault.h
 *
 *  Created on: May 14 2022
 *      Author: Liam Flaherty
 */

#include "faultManager.h"
#include <math.h>

// ------------------- Private data -------------------
static Logging_T* mLog;

// ------------------- Private methods -------------------
/**
 * @brief Handle a timed check of sampled data
 *
 * @param isFault true for fault in current sample, false if current sample ok.
 * @param timer Timer variable to increment.
 * @param limit Limit for timer
 * @return true If timed check is valid
 * @return false If timed check failed
 */
static bool handleTimedCondition(bool isFault, uint16_t* timer, uint16_t limit)
{
  bool result = true;
  if (isFault) {
    (*timer)++;

    if (*timer > limit) {
      *timer = limit; // prevent overflow
      result = false;
    }
  } else {
    // Reset timer
    *timer = 0U;
  }

  return result;
}

static uint32_t isFaultAccelPedal(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  uint32_t faults = faultMgr->internal.faults;

  // Accelerator outside of calibrated range
  uint16_t accelRawA = data->inputs.accelRawA;
  uint16_t accelRawALimitUpper = faultMgr->vehicleConfig->inputs.accelPedal.calibrationA.rawUpper;
  uint16_t accelRawALimitLower = faultMgr->vehicleConfig->inputs.accelPedal.calibrationA.rawLower;
  uint16_t accelRawB = data->inputs.accelRawB;
  uint16_t accelRawBLimitUpper = faultMgr->vehicleConfig->inputs.accelPedal.calibrationB.rawUpper;
  uint16_t accelRawBLimitLower = faultMgr->vehicleConfig->inputs.accelPedal.calibrationB.rawLower;

  bool accelRangeCheck = handleTimedCondition(
      accelRawA > accelRawALimitUpper || accelRawA < accelRawALimitLower ||
      accelRawB > accelRawBLimitUpper || accelRawB < accelRawBLimitLower,
      &faultMgr->internal.accelPedalRangeTimer,
      faultMgr->internal.accelPedalRangeTimerLimit);
  if (!accelRangeCheck) {
    faults |= FAULTMGR_FAULT_ACCELPDL_RANGE;
  }

  // Redundant values disagree
  float diff = fabs(data->inputs.accelA - data->inputs.accelB);
  bool diffCheck = handleTimedCondition(
      diff > faultMgr->vehicleConfig->inputs.accelPedal.consistencyLimit,
      &faultMgr->internal.pedalConsistencyTimer,
      faultMgr->internal.pedalConsistencyTimerLimit
  );
  if (!diffCheck) {
    faults |= FAULTMGR_FAULT_ACCELPDL_SENSE;
  }

  return faults;
}

static uint32_t isFaultBrakePedal(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  (void)faultMgr;
  (void)data;
  return 0x0U;
}

static uint32_t isFaultBMS(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  (void)faultMgr;
  (void)data;
  return 0x0U;
}

static uint32_t isFaultInverter(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  (void)faultMgr;
  (void)data;
  return 0x0U;
}

static uint32_t isLVErrorBMS(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  (void)faultMgr;
  (void)data;
  return 0x0U;
}

static uint32_t isLVErrorInverter(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  (void)faultMgr;
  (void)data;
  return 0x0U;
}

// ------------------- Public methods -------------------
void FaultManager_Init(Logging_T* logger, FaultManager_T* faultMgr)
{
  mLog = logger;
  logPrintS(mLog, "FaultManager_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  memset(&faultMgr->internal, 0U, sizeof(FaultManager_Internal_T));

  // Re-calculate fault timer/counter limits
  faultMgr->internal.accelPedalRangeTimerLimit =
      faultMgr->vehicleConfig->inputs.accelPedal.invalidDataTimeout / faultMgr->tickRateMs;
  faultMgr->internal.pedalConsistencyTimerLimit =
      faultMgr->vehicleConfig->inputs.accelPedal.invalidDataTimeout / faultMgr->tickRateMs;

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
  faultStatus |= isFaultAccelPedal(faultMgr, &latestData);
  faultStatus |= isFaultBrakePedal(faultMgr, &latestData);
  faultStatus |= isFaultBMS(faultMgr, &latestData);
  faultStatus |= isFaultInverter(faultMgr, &latestData);
  faultStatus |= isLVErrorBMS(faultMgr, &latestData);
  faultStatus |= isLVErrorInverter(faultMgr, &latestData);

  // latch faults
  faultMgr->internal.faults |= faultStatus;

  if ((faultStatus & FAULTMGR_FAULT_MASK) > 0U) {
    return FAULT_FAULT;
  } else if ((faultStatus & FAULTMGR_LV_ERROR_MASK) > 0U) {
    return FAULT_LV_ERROR;
  } else {
    return FAULT_NO_FAULT;
  }
}