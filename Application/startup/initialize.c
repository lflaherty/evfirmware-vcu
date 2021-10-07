/*
 * initialize.c
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#include "initialize.h"

#include <stdio.h>
#include <stdbool.h>

#include "lib/logging/logging.h"
#include "comm/can/can.h"
#include "comm/uart/uart.h"
#include "io/adc/adc.h"
#include "time/tasktimer/tasktimer.h"
#include "time/externalWatchdog/externalWatchdog.h"
#include "time/rtc/rtc.h"

#include "vehicleInterface/deviceMapping/deviceMapping.h"

// ------------------- Private data -------------------
static Logging_T mLog;

// ------------------- Module structures -------------------
static ExternalWatchdog_T mWdg;

// ------------------- Private prototypes -------------------
static ECU_Init_Status_T ECU_Init_System1(void);  // Init basics for logging
static ECU_Init_Status_T ECU_Init_System2(void);  // Init remaining internal peripherals
static ECU_Init_Status_T ECU_Init_System3(void);  // Init external peripherals
static ECU_Init_Status_T ECU_Init_App1(void);     // Init application devices
static ECU_Init_Status_T ECU_Init_App2(void);     // Init application vehicle interface
static ECU_Init_Status_T ECU_Init_App3(void);     // Init application processes

//------------------------------------------------------------------------------
ECU_Init_Status_T ECU_Init(void)
{
  ECU_Init_Status_T ret = ECU_INIT_OK;

  // Initialize components
  ret |= ECU_Init_System1();
  ret |= ECU_Init_System2();
  ret |= ECU_Init_System3();
  ret |= ECU_Init_App1();
  ret |= ECU_Init_App2();
  ret |= ECU_Init_App3();

  if (ret != ECU_INIT_OK) {
    logPrintS(&mLog, "Failed to initialize\n", LOGGING_DEFAULT_BUFF_LEN);
    return ret;
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

  // External watchdog
  mWdg.gpioBank = External_WDG_Trigger_GPIO_Port;
  mWdg.gpioPin = External_WDG_Trigger_Pin;
  if (ExternalWatchdog_Init(&mLog, &mWdg) != EXTWATCHDOG_STATUS_OK) {
    logPrintS(&mLog, "ExternalWatchdog initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App1(void)
{
  logPrintS(&mLog, "###### ECU_Init_App1 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App2(void)
{
  logPrintS(&mLog, "###### ECU_Init_App2 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App3(void)
{
  logPrintS(&mLog, "###### ECU_Init_App3 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

