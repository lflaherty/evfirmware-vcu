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
#include "comm/can/can.h"

#include "vehicleInterface/config/deviceMapping.h"
#include "vehicleInterface/config/configData.h"
#include "vehicleInterface/vehicleState/vehicleState.h"
#include "vehicleInterface/vehicleControl/vehicleControl.h"

#include "device/gps/gps.h"
#include "device/sdc/sdc.h"
#include "device/pdm/pdm.h"
#include "device/pcinterface/pcinterface.h"
#include "device/inverter/cInverter.h"
#include "device/wheelspeed/wheelspeed.h"

#include "vehicleLogic/watchdogTrigger/watchdogTrigger.h"


// ------------------- Private data -------------------
#define INIT_STACK_SIZE 1500U
#define INIT_TASK_PRIORITY 15U
struct InitTask {
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[INIT_STACK_SIZE];
};
static struct InitTask initTask;

extern const char* welcomeMsg;

// ------------------- Module structures -------------------
// MCU peripherals
static Logging_T mLog;
static CRC_T mCrc;

// ECU peripherals
static GPS_T mGps;
static WatchdogTrigger_T mWdtTrigger;
static PCInterface_T mPCInterface;
static PDM_T mPdm;
static CInverter_T mInverter;

// Vehicle interface
static Config_T mConfig;
static VehicleState_T mVehicleState;
static VehicleControl_T mVehicleControl;

// Vehicle processes
// TODO

// ------------------- Private methods -------------------
/**
 * @brief RTOS task method for init
 * 
 * @param pvParameters NULL
 */
static void ECU_Init_Task(void* pvParameters);

/**
 * @brief Init basics for logging and running modules
 */
static void ECU_Init_System(void);

/**
 * @brief Init remaining internal peripherals
 */
static void ECU_Init_BoardPeriph(void);

/**
 * @brief Init external peripherals
 */
static void ECU_Init_BoardDevs(void);

/**
 * @brief Load config parameters from EEPROM
 */
static void ECU_Init_LoadConfig(void);

/**
 * @brief Init application vehicle interface
 * Runs before vehicle device init.
 * Devices depend on this to push data.
 */
static void ECU_Init_VehicleInterface1(void);

/**
 * @brief Init drivers for devices connected to ECU
 */
static void ECU_Init_VehicleDevices(void);

/**
 * @brief Init application vehicle interface.
 * Runs after vehicle device init.
 * Modules here depend on devices to control vehicle.
 */
static void ECU_Init_VehicleInterface2(void);

/**
 * @brief Init vehicle application processes
 */
static void ECU_Init_VehicleProccesses(void);

/**
 * @brief Invoked if initialization failed.
 * 
 */
static void ECU_Init_Error(const char* msg);

//------------------------------------------------------------------------------
void ECU_Init_Error(const char* msg)
{
  Log_Print(&mLog, msg);
  Log_Print(&mLog, "ECU Failed to initialize\n");

  // TODO should flash LED here and assert ECU fault pin

  while (1);
}

//------------------------------------------------------------------------------
void ECU_Init(const InitData_T* initData)
{
  // Add init task
  initTask.taskHandle = xTaskCreateStatic(
      ECU_Init_Task,
      "init",
      INIT_STACK_SIZE,
      (void*)initData,
      INIT_TASK_PRIORITY,
      initTask.taskStack,
      &initTask.taskBuffer);

  // Start RTOS
  vTaskStartScheduler();
}

//------------------------------------------------------------------------------
void ECU_Init_Task(void* pvParameters)
{
  const InitData_T* initData = (const InitData_T*)pvParameters;
  (void)initData;
  // TODO use initData

  // Initialize components
  ECU_Init_System();

  Log_Print(&mLog, "\n\n\n");
  Log_Print(&mLog, welcomeMsg);

  ECU_Init_BoardPeriph();
  ECU_Init_BoardDevs();
  ECU_Init_LoadConfig();
  ECU_Init_VehicleInterface1();
  ECU_Init_VehicleDevices();
  ECU_Init_VehicleInterface2();
  ECU_Init_VehicleProccesses();

  Log_Print(&mLog, "ECU_Init complete\n");
  Log_Print(&mLog, "ECU_Init deleting init task\n");
  vTaskDelete(NULL);
}

//------------------------------------------------------------------------------
static void ECU_Init_System(void)
{
  // Set up logging
  Logging_Status_T statusLog;
  statusLog = Log_Init(&mLog);
  if (LOGGING_STATUS_OK != statusLog) {
    ECU_Init_Error("Log_Init error\n");
  }

  Log_Print(&mLog, "###### ECU_Init_System ######\n");

  // Timers
  if (TaskTimer_Init(&mLog, Mapping_GetTaskTimer100Hz()) != TASKTIMER_STATUS_OK) {
    Log_Print(&mLog, "Task Timer 100Hz initialization error\n");
  }

  // statusLog = Log_EnableSWO(&mLog);
  // if (LOGGING_STATUS_OK != statusLog) {
  //   ECU_Init_Error("Log_EnableSWO error\n");
  // }

  // UART
  UART_Status_T statusUart;
  statusUart = UART_Init(&mLog);
  if (UART_STATUS_OK != statusUart) {
    ECU_Init_Error("UART initialization error\n");
  }

  statusUart = UART_Config(&Mapping_PCInterface_UARTA);
  if (UART_STATUS_OK != statusUart) {
    ECU_Init_Error("USART1 (PCDebug A) config error\n");
  }

  statusUart = UART_Config(&Mapping_PCInterface_UARTB);
  if (UART_STATUS_OK != statusUart) {
    ECU_Init_Error("USART3 (PCDebug B) config error\n");
  }

  // TODO move this to GPS section
  statusUart = UART_Config(&Mapping_GPS_UART);
  if (UART_STATUS_OK != statusUart) {
    ECU_Init_Error("USART6 (GPS) config error\n");
  }

  // CRC
  mCrc.hcrc = Mapping_GetCRC();
  CRC_Status_T statusCrc;
  statusCrc = CRC_Init(&mLog, &mCrc);
  if (CRC_STATUS_OK != statusCrc) {
    ECU_Init_Error("CRC initialization error\n");
  }

  // Create PC Debug driver
  // Create it this early so we get serial output, we'll enable the other
  // functionality later on
  mPCInterface.uartA = MAPPING_PCINTERFACE_UARTADEV;
  mPCInterface.uartB = MAPPING_PCINTERFACE_UARTBDEV;
  mPCInterface.crc = &mCrc;
  mPCInterface.pinToggle = &Mapping_GPO_DebugToggle;
  PCInterface_Status_T statusPcInterface = PCInterface_Init(&mLog, &mPCInterface);
  if (PCINTERFACE_STATUS_OK != statusPcInterface) {
    ECU_Init_Error("PCDebug initialization error\n");
  }
}

//------------------------------------------------------------------------------
static void ECU_Init_BoardPeriph(void)
{
  Log_Print(&mLog, "###### ECU_Init_BoardPeriph ######\n");

  // CAN bus
  CAN_Status_T statusCan;
  statusCan = CAN_Init(&mLog);
  if (CAN_STATUS_OK != statusCan) {
    ECU_Init_Error("CAN initialization error\n");
  }

  statusCan = CAN_Config(CAN_DEV1, Mapping_GetCAN1());
  if (CAN_STATUS_OK != statusCan) {
    ECU_Init_Error("CAN1 config error\n");
  }

  mPCInterface.canDebugEnable = true; // TODO remove

  // ADC
  // Not configured in this release

  // RTC
  // Not configured in this release
}

//------------------------------------------------------------------------------
static void ECU_Init_BoardDevs(void)
{
  Log_Print(&mLog, "###### ECU_Init_BoardDevs ######\n");
}

//------------------------------------------------------------------------------
static void ECU_Init_LoadConfig(void)
{
  memset(&mConfig, 0, sizeof(mConfig));

  // TODO placeholder, update to load from EEPROM
  mConfig.inputs.numWheelspeedTeeth = 12;
}

//------------------------------------------------------------------------------
static void ECU_Init_VehicleInterface1(void)
{
  Log_Print(&mLog, "###### ECU_Init_VehicleInterface1 ######\n");

  // Vehicle state
  if (VehicleState_Init(&mLog, &mVehicleState) != VEHICLESTATE_STATUS_OK) {
    ECU_Init_Error("VehicleState initialization error\n");
  }

  if (PCInterface_SetVehicleState(&mPCInterface, &mVehicleState) != PCINTERFACE_STATUS_OK) {
    ECU_Init_Error("PCInterface_SetVehicleState failed\n");
  }
}

//------------------------------------------------------------------------------
static void ECU_Init_VehicleDevices(void)
{
  Log_Print(&mLog, "###### ECU_Init_VehicleDevices ######\n");

  // GPS
  mGps.pin3dFix = &Mapping_GPS_3dFixPin;
  mGps.uart = MAPPING_GPS_UARTDEV;
  mGps.state = &mVehicleState;
  if (GPS_Init(&mLog, &mGps) != GPS_STATUS_OK) {
    ECU_Init_Error("GPS initialization error\n");
  }

  // Wheel speed
  Wheelspeed_Config_T wsConfig = {
    .logging = &mLog,
    .state = &mVehicleState,
    .frontWsPin = &Mapping_GPI_Wheelspeed_Front,
    .rearWsPin = &Mapping_GPI_Wheelspeed_Rear,
    .timerInstance = Mapping_GetTaskTimer2kHz()->Instance,
    .sensorTeeth = mConfig.inputs.numWheelspeedTeeth,
  };
  if (Wheelspeed_Init(&wsConfig) != WHEELSPEED_STATUS_OK) {
    ECU_Init_Error("Wheelspeed initialization error\n");
  }

  // Shutdown circuit
  SDC_Config_T sdcConfig = {
    .state = &mVehicleState,
    .pinBMS = &Mapping_GPI_SDC_BMS,
    .pinBSPD = &Mapping_GPI_SDC_BSPD,
    .pinIMD = &Mapping_GPI_SDC_IMD,
    .pinSDCOut = &Mapping_GPI_SDC_SDCOut,
    .pinECUError = &Mapping_GPO_SDC_ECUError,
  };
  if (SDC_Init(&mLog, &sdcConfig) != SDC_STATUS_OK) {
    ECU_Init_Error("SDC initialization error\n");
  }

  // Power distribution module (PDM)
  mPdm.channels = pdmChannels;
  mPdm.numChannels = numPdmChannels;
  mPdm.vehicleState = &mVehicleState;
  if (PDM_Init(&mLog, &mPdm) != PDM_STATUS_OK) {
    ECU_Init_Error("PDM initialization error\n");
  }

  // Inverter
  mInverter.canInst = CAN_DEV1;
  mInverter.vehicleState = &mVehicleState;
  if (CInverter_Init(&mLog, &mInverter) != CINVERTER_STATUS_OK) {
    ECU_Init_Error("Inverter initialization error\n");
  }
}

//------------------------------------------------------------------------------
static void ECU_Init_VehicleInterface2(void)
{
  Log_Print(&mLog, "###### ECU_Init_VehicleInterface2 ######\n");

  // Vehicle control
  mVehicleControl.pdm = &mPdm;
  mVehicleControl.inverter = &mInverter;
  if (VehicleControl_Init(&mLog, &mVehicleControl)) {
    ECU_Init_Error("VehicleControl initialization error\n");
  }
  if (PCInterface_SetVehicleControl(&mPCInterface, &mVehicleControl) != PCINTERFACE_STATUS_OK) {
    ECU_Init_Error("PCInterface_SetVehicleControl error\n");
  }
}

//------------------------------------------------------------------------------
static void ECU_Init_VehicleProccesses(void)
{
  Log_Print(&mLog, "###### ECU_Init_VehicleProccesses ######\n");

  // Watchdog Trigger
  mWdtTrigger.blinkLED = &Mapping_GPO_LED;
  if (WatchdogTrigger_Init(&mLog, &mWdtTrigger) != WATCHDOGTRIGGER_STATUS_OK) {
    ECU_Init_Error("Watchdog Trigger initialization error\n");
  }
}

// TODO move this somewhere more appropriate
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  SDC_IRQHandler(GPIO_Pin);
}

void TIM_IRQHandler(TIM_HandleTypeDef *htim)
{
  // Advance the TaskTimer
  TaskTimer_TIM_PeriodElapsedCallback(htim);

  // Invoke wheel speed handler
  Wheelspeed_TIM_IRQHandler(htim);
}
