/*
 * fault.h
 *
 *  Created on: May 14 2022
 *      Author: Liam Flaherty
 */

#include "faultManager.h"
#include <math.h>
#include <string.h>

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
  float diff = fabsf(data->inputs.accelA - data->inputs.accelB);
  bool diffCheck = handleTimedCondition(
      diff > faultMgr->vehicleConfig->inputs.accelPedal.consistencyLimit,
      &faultMgr->internal.accelPedalConsistencyTimer,
      faultMgr->internal.accelPedalConsistencyTimerLimit
  );
  if (!diffCheck) {
    faults |= FAULTMGR_FAULT_ACCELPDL_SENSE;
  }

  return faults;
}

static uint32_t isFaultBrakePedal(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  uint32_t faults = faultMgr->internal.faults;

  // Brake pressure outside of calibrated range
  uint16_t brakePresFront = data->inputs.brakeRawFront;
  uint16_t brakePresFrontLimitUpper = faultMgr->vehicleConfig->inputs.brakePressureFront.rawUpper;
  uint16_t brakePresFrontLimitLower = faultMgr->vehicleConfig->inputs.brakePressureFront.rawLower;
  uint16_t brakeRawRear = data->inputs.brakeRawRear;
  uint16_t brakeRawRearLimitUpper = faultMgr->vehicleConfig->inputs.brakePressureRear.rawUpper;
  uint16_t brakeRawRearLimitLower = faultMgr->vehicleConfig->inputs.brakePressureRear.rawLower;

  bool brakeRangeCheck = handleTimedCondition(
      brakePresFront > brakePresFrontLimitUpper || brakePresFront < brakePresFrontLimitLower ||
      brakeRawRear > brakeRawRearLimitUpper || brakeRawRear < brakeRawRearLimitLower,
      &faultMgr->internal.brakePressureRangeTimer,
      faultMgr->internal.brakePressureRangeTimerLimit);
  if (!brakeRangeCheck) {
    faults |= FAULTMGR_FAULT_BRAKEPDL_RANGE;
  }

  // Accel-brake pedal abuse
  if (1U == faultMgr->vehicleConfig->inputs.pedalAbuseCheckEnabled) {
    bool pedalAbuseCondition =
        data->inputs.accel > faultMgr->vehicleConfig->inputs.pedalAbuseAccelThreshold &&
        data->inputs.brakePresFront > faultMgr->vehicleConfig->inputs.pedalAbuseBrakeThreshold;
    bool pedalAbuseCheck = handleTimedCondition(
        pedalAbuseCondition,
        &faultMgr->internal.pedalAbuseTimer,
        faultMgr->internal.pedalAbuseTimerLimit
    );
    if (!pedalAbuseCheck) {
      faults |= FAULTMGR_FAULT_BRAKEPDL_ABUSE;
    }
  }

  return faults;
}

static uint32_t isFaultBMS(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  uint32_t faults = faultMgr->internal.faults;

  // Cell temperature over threshold
  bool cellTempOverCondition =
      data->battery.maxCellTemperature > faultMgr->vehicleConfig->bms.maxCellTemp;
  bool cellTempOverCheck = handleTimedCondition(
      cellTempOverCondition,
      &faultMgr->internal.cellTempOverTimer,
      faultMgr->internal.bmsFaultTimerLimit
  );
  if (!cellTempOverCheck) {
    faults |= FAULTMGR_FAULT_BMS_CELLTEMP;
  }

  // battery current draw too high
  bool currentOverDrawCondition =
      data->battery.dcCurrent > faultMgr->vehicleConfig->bms.maxCurrent;
  bool currentOverDrawCheck = handleTimedCondition(
      currentOverDrawCondition,
      &faultMgr->internal.currentOverDrawTimer,
      faultMgr->internal.bmsFaultTimerLimit
  );
  if (!currentOverDrawCheck) {
    faults |= FAULTMGR_FAULT_BMS_CURRENT;
  }

  // Cell voltage over threshold
  bool cellVoltageOverCondition =
      data->battery.maxCellVoltage > faultMgr->vehicleConfig->bms.maxCellVoltage;
  bool cellVoltageOverCheck = handleTimedCondition(
      cellVoltageOverCondition,
      &faultMgr->internal.cellVoltageOverTimer,
      faultMgr->internal.bmsFaultTimerLimit
  );
  if (!cellVoltageOverCheck) {
    faults |= FAULTMGR_FAULT_BMS_CELLVOLT;
  }

  // state of charge too low
  bool chargeTooLow =
      data->battery.stateOfCarge < faultMgr->vehicleConfig->bms.minStateOfCharge;
  if (chargeTooLow) {
    faults |= FAULTMGR_FAULT_BMS_LOWSOC;
  }

  // BMS fault indicator
  if (data->battery.bmsFaultIndicator) {
    faults |= FAULTMGR_FAULT_BMS_FAULTIND;
  }

  return faults;
}

static uint32_t isFaultInverter(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  (void)faultMgr;
  (void)data;
  // TODO
  return 0x0U;
}

static uint32_t isLVErrorBMS(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  (void)faultMgr;
  (void)data;
  // TODO
  return 0x0U;
}

static uint32_t isLVErrorInverter(FaultManager_T* faultMgr, VehicleState_Data_T* data)
{
  (void)faultMgr;
  (void)data;
  // TODO
  return 0x0U;
}

// ------------------- Public methods -------------------
void FaultManager_Init(Logging_T* logger, FaultManager_T* faultMgr)
{
  mLog = logger;
  Log_Print(mLog, "FaultManager_Init begin\n");

  memset(&faultMgr->internal, 0U, sizeof(FaultManager_Internal_T));

  // Re-calculate fault timer/counter limits
  faultMgr->internal.accelPedalRangeTimerLimit =
      (uint16_t)(faultMgr->vehicleConfig->inputs.invalidDataTimeout / faultMgr->tickRateMs);
  faultMgr->internal.accelPedalConsistencyTimerLimit =
      (uint16_t)(faultMgr->vehicleConfig->inputs.invalidDataTimeout / faultMgr->tickRateMs);
  faultMgr->internal.brakePressureRangeTimerLimit =
      (uint16_t)(faultMgr->vehicleConfig->inputs.invalidDataTimeout / faultMgr->tickRateMs);
  faultMgr->internal.brakePressureConsistencyTimerLimit =
      (uint16_t)(faultMgr->vehicleConfig->inputs.invalidDataTimeout / faultMgr->tickRateMs);
  faultMgr->internal.pedalAbuseTimerLimit =
      (uint16_t)(faultMgr->vehicleConfig->inputs.invalidDataTimeout / faultMgr->tickRateMs);
  faultMgr->internal.bmsFaultTimerLimit =
      (uint16_t)(faultMgr->vehicleConfig->bms.invalidDataTimeout / faultMgr->tickRateMs);

  Log_Print(mLog, "FaultManager_Init complete\n");
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
  faultMgr->internal.faults |= isFaultAccelPedal(faultMgr, &latestData);
  faultMgr->internal.faults |= isFaultBrakePedal(faultMgr, &latestData);
  faultMgr->internal.faults |= isFaultBMS(faultMgr, &latestData);
  faultMgr->internal.faults |= isFaultInverter(faultMgr, &latestData);
  faultMgr->internal.faults |= isLVErrorBMS(faultMgr, &latestData);
  faultMgr->internal.faults |= isLVErrorInverter(faultMgr, &latestData);

  if ((faultMgr->internal.faults & FAULTMGR_FAULT_MASK) > 0U) {
    return FAULT_FAULT;
  } else if ((faultMgr->internal.faults & FAULTMGR_LV_ERROR_MASK) > 0U) {
    return FAULT_LV_ERROR;
  } else {
    return FAULT_NO_FAULT;
  }
}
