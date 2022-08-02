/*
 * initialize.c
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#include "initialize.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>  // for memset

#include "lib/logging/logging.h"
#include "comm/can/can.h"
#include "comm/uart/uart.h"
#include "io/adc/adc.h"
#include "time/tasktimer/tasktimer.h"
#include "time/rtc/rtc.h"

#include "device/inverter/cInverter.h"
#include "vehicleInterface/config/deviceMapping.h"
#include "vehicleInterface/vehicleState/vehicleState.h"

// ------------------- Private data -------------------
static Logging_T mLog;

// ------------------- Module structures -------------------
static WatchdogTrigger_T mWdtTrigger;

// ------------------- Private prototypes -------------------
static ECU_Init_Status_T ECU_Init_System1(void);  // Init basics for logging
static ECU_Init_Status_T ECU_Init_System2(void);  // Init remaining internal peripherals
static ECU_Init_Status_T ECU_Init_System3(void);  // Init external peripherals
static ECU_Init_Status_T ECU_Init_App1(void);     // Init application vehicle interface (devices depends on this to push data)
static ECU_Init_Status_T ECU_Init_App2(void);     // Init application devices
static ECU_Init_Status_T ECU_Init_App3(void);     // Init application processes
static void ECU_Init_Hang(void);

//------------------------------------------------------------------------------
void ECU_Init_Hang(void)
{
  logPrintS(&mLog, "ECU Failed to initialize\n", LOGGING_DEFAULT_BUFF_LEN);

  while (1);
}

//------------------------------------------------------------------------------
ECU_Init_Status_T ECU_Init(void)
{
  // Initialize components
  if (ECU_INIT_OK != ECU_Init_System1()) {
    ECU_Init_Hang();
  }
  if (ECU_INIT_OK != ECU_Init_System2()) {
    ECU_Init_Hang();
  }
  if (ECU_INIT_OK != ECU_Init_System3()) {
    ECU_Init_Hang();
  }
  if (ECU_INIT_OK != ECU_Init_App1()) {
    ECU_Init_Hang();
  }
  if (ECU_INIT_OK != ECU_Init_App2()) {
    ECU_Init_Hang();
  }
  if (ECU_INIT_OK != ECU_Init_App3()) {
    ECU_Init_Hang();
  }

  logPrintS(&mLog, "ECU_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System1(void)
{
  // Set up logging
  mLog.enableLogToDebug = true;
  mLog.enableLogToSerial = false;
  mLog.enableLogToLogFile = false;
  mLog.handleSerial = NULL;

  logPrintS(&mLog, "###### ECU_Init_System1 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // UART
  if (UART_Init(&mLog) != UART_STATUS_OK) {
    logPrintS(&mLog, "UART initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  if (UART_Config(Mapping_GetUART1()) != UART_STATUS_OK) {
    logPrintS(&mLog, "UART config error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // enable serial logging
  mLog.enableLogToSerial = true;
  mLog.handleSerial = Mapping_GetUART1();

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System2(void)
{
  logPrintS(&mLog, "###### ECU_Init_System2 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // CAN bus
  if (CAN_Init(&mLog) != CAN_STATUS_OK) {
    logPrintS(&mLog, "CAN initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  if (CAN_Config(Mapping_GetCAN1()) != CAN_STATUS_OK) {
    logPrintS(&mLog, "CAN1 config error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  if (CAN_Config(Mapping_GetCAN2()) != CAN_STATUS_OK) {
    logPrintS(&mLog, "CAN2 config error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  if (CAN_Config(Mapping_GetCAN3()) != CAN_STATUS_OK) {
    logPrintS(&mLog, "CAN3 config error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // ADC
  if (ADC_Init(&mLog, MAPPING_ADC_NUM_CHANNELS, 16) != ADC_STATUS_OK) {
    logPrintS(&mLog, "ADC initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  if (ADC_Config(Mapping_GetADC()) != ADC_STATUS_OK) {
    logPrintS(&mLog, "ADC config error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // Timers
  if (TaskTimer_Init(&mLog, Mapping_GetTaskTimer()) != TASKTIMER_STATUS_OK) {
    logPrintS(&mLog, "Task Timer initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // RTC
  if (RTC_Init(&mLog) != RTC_STATUS_OK) {
    logPrintS(&mLog, "RTC initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System3(void)
{
  logPrintS(&mLog, "###### ECU_Init_System3 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App1(void)
{
  logPrintS(&mLog, "###### ECU_Init_App2 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  memset(&mVehicleState, 0, sizeof(mVehicleState));
  if (VehicleState_Init(&mLog, &mVehicleState) != VEHICLESTATE_STATUS_OK) {
    logPrintS(&mLog, "VehicleState initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App2(void)
{
  logPrintS(&mLog, "###### ECU_Init_App1 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // Inverter
  memset(&mInverter, 0, sizeof(mInverter));
  mInverter.hcan = Mapping_GetCAN1();
  mInverter.canIdBase = 0;
  mInverter.vehicleState = &mVehicleState;
  if (CInverter_Init(&mLog, &mInverter) != CINVERTER_STATUS_OK) {
    logPrintS(&mLog, "Inverter initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App3(void)
{
  logPrintS(&mLog, "###### ECU_Init_App3 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // Watchdog Trigger
  mWdtTrigger.blinkLED = &Mapping_GPIO_LED;
  if (WatchdogTrigger_Init(&mLog, &mWdtTrigger) != WATCHDOGTRIGGER_STATUS_OK) {
    logPrintS(&mLog, "Watchdog Trigger initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

