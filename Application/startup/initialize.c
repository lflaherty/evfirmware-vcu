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
static Logging_T log;

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
    logPrintS(&log, "Failed to initialize\n", LOGGING_DEFAULT_BUFF_LEN);
    return ret;
  }

  logPrintS(&log, "ECU_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System1(void)
{
  // Set up logging
  log.enableLogToDebug = true;
  log.enableLogToSerial = false;
  log.enableLogToLogFile = false;
  log.handleSerial = NULL;

  logPrintS(&log, "###### ECU_Init_System1 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // UART
  if (UART_Init(&log) != UART_STATUS_OK) {
    logPrintS(&log, "UART initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  if (UART_Config(Mapping_GetUART1()) != UART_STATUS_OK) {
    logPrintS(&log, "UART config error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  // enable serial logging
  log.enableLogToSerial = true;
  log.handleSerial = Mapping_GetUART1();

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System2(void)
{
  logPrintS(&log, "###### ECU_Init_System2 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  // CAN bus
  if (CAN_Init(&log) != CAN_STATUS_OK) {
    logPrintS(&log, "CAN initialization error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  if (CAN_Config(Mapping_GetCAN1()) != CAN_STATUS_OK) {
    logPrintS(&log, "CAN1 config error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  if (CAN_Config(Mapping_GetCAN2()) != CAN_STATUS_OK) {
    logPrintS(&log, "CAN2 config error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  if (CAN_Config(Mapping_GetCAN3()) != CAN_STATUS_OK) {
    logPrintS(&log, "CAN3 config error\n", LOGGING_DEFAULT_BUFF_LEN);
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System3(void)
{
  logPrintS(&log, "###### ECU_Init_System3 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App1(void)
{
  logPrintS(&log, "###### ECU_Init_App1 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App2(void)
{
  logPrintS(&log, "###### ECU_Init_App2 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App3(void)
{
  logPrintS(&log, "###### ECU_Init_App3 ######\n", LOGGING_DEFAULT_BUFF_LEN);

  return ECU_INIT_OK;
}

