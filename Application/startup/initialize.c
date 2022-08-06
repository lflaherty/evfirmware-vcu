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
#include "time/tasktimer/tasktimer.h"

#include "vehicleInterface/config/deviceMapping.h"

#include "vehicleLogic/watchdogTrigger/watchdogTrigger.h"

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
  Logging_Status_T statusLog;
  statusLog = Log_Init(&mLog);
  if (LOGGING_STATUS_OK != statusLog) {
    return ECU_INIT_ERROR;
  }

  statusLog = Log_EnableSWO(&mLog);
  if (LOGGING_STATUS_OK != statusLog) {
    return ECU_INIT_ERROR;
  }

  logPrintS(&mLog, "###### ECU_Init_System1 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // UART
  // Not configured in this release

  // enable serial logging
  // TODO

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System2(void)
{
  logPrintS(&mLog, "###### ECU_Init_System2 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // CAN bus
  // Not configured in this release

  // ADC
  // Not configured in this release

  // Timers
  if (TaskTimer_Init(&mLog, Mapping_GetTaskTimer()) != TASKTIMER_STATUS_OK) {
    logPrintS(&mLog, "Task Timer initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // RTC
  // Not configured in this release

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System3(void)
{
  logPrintS(&mLog, "###### ECU_Init_System3 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // Not configured in this release

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App1(void)
{
  logPrintS(&mLog, "###### ECU_Init_App2 ######\n", LOGGING_DEFAULT_BUFF_LEN);
  
  // Not configured in this release

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App2(void)
{
  logPrintS(&mLog, "###### ECU_Init_App1 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // Not configured in this release

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
