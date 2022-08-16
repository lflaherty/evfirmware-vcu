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
#include "comm/uart/uart.h"

#include "vehicleInterface/config/deviceMapping.h"

#include "device/pcdebug/pcdebug.h"

#include "vehicleLogic/watchdogTrigger/watchdogTrigger.h"


typedef enum
{
  ECU_INIT_OK     = 0x00U,
  ECU_INIT_ERROR  = 0x01
} ECU_Init_Status_T;

// ------------------- Private data -------------------
static Logging_T mLog;

#define INIT_STACK_SIZE 1500U
#define INIT_TASK_PRIORITY 15U
struct InitTask {
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[INIT_STACK_SIZE];
};
static struct InitTask initTask;

// ------------------- Module structures -------------------
static WatchdogTrigger_T mWdtTrigger;
static PCDebug_T mPCDebug;

// ------------------- Private prototypes -------------------
static void ECU_Init_Task(void* pvParameters);  // RTOS task method for init
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
  Log_Print(&mLog, "ECU Failed to initialize\n");

  while (1);
}

//------------------------------------------------------------------------------
void ECU_Init(void)
{
  // Add init task
  initTask.taskHandle = xTaskCreateStatic(
      ECU_Init_Task,
      "init",
      INIT_STACK_SIZE,
      NULL,
      INIT_TASK_PRIORITY,
      initTask.taskStack,
      &initTask.taskBuffer);

  // Start RTOS
  vTaskStartScheduler();
}

//------------------------------------------------------------------------------
void ECU_Init_Task(void* pvParameters)
{
  (void)pvParameters;

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

  Log_Print(&mLog, "ECU_Init complete\n");
  Log_Print(&mLog, "ECU_Init deleting init task\n");
  vTaskDelete(NULL);
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

  Log_Print(&mLog, "###### ECU_Init_System1 ######\n");

  // UART
  UART_Status_T statusUart;
  statusUart = UART_Init(&mLog);
  if (UART_STATUS_OK != statusUart) {
    Log_Print(&mLog, "UART initialization error\n");
    return ECU_INIT_ERROR;
  }

  statusUart = UART_Config(Mapping_GetPCDebugUart());
  if (UART_STATUS_OK != statusUart) {
    Log_Print(&mLog, "USART1 (PCDebug) config error\n");
    return ECU_INIT_ERROR;
  }

  // Create PC Debug driver
  mPCDebug.huart = Mapping_GetPCDebugUart();
  mPCDebug.hcrc = Mapping_GetCRC();
  PCDebug_Status_T statusPcDebug = PCDebug_Init(&mLog, &mPCDebug);
  if (PCDEBUG_STATUS_OK != statusPcDebug) {
    Log_Print(&mLog, "PCDebug initialization error\n");
    return ECU_INIT_ERROR;
  }

  Log_Print(&mLog, "Serial logging enabled\n");

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System2(void)
{
  Log_Print(&mLog, "###### ECU_Init_System2 ######\n");

  // CAN bus
  // Not configured in this release

  // ADC
  // Not configured in this release

  // Timers
  if (TaskTimer_Init(&mLog, Mapping_GetTaskTimer()) != TASKTIMER_STATUS_OK) {
    Log_Print(&mLog, "Task Timer initialization error\n");
    return ECU_INIT_ERROR;
  }

  // RTC
  // Not configured in this release

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_System3(void)
{
  Log_Print(&mLog, "###### ECU_Init_System3 ######\n");

  // Not configured in this release

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App1(void)
{
  Log_Print(&mLog, "###### ECU_Init_App2 ######\n");
  
  // Not configured in this release

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App2(void)
{
  Log_Print(&mLog, "###### ECU_Init_App1 ######\n");

  // Not configured in this release

  return ECU_INIT_OK;
}

//------------------------------------------------------------------------------
static ECU_Init_Status_T ECU_Init_App3(void)
{
  Log_Print(&mLog, "###### ECU_Init_App3 ######\n");

  // Watchdog Trigger
  mWdtTrigger.blinkLED = &Mapping_GPIO_LED;
  if (WatchdogTrigger_Init(&mLog, &mWdtTrigger) != WATCHDOGTRIGGER_STATUS_OK) {
    Log_Print(&mLog, "Watchdog Trigger initialization error\n");
    return ECU_INIT_ERROR;
  }

  return ECU_INIT_OK;
}
