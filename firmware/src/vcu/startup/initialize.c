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

#include "logging/logging.h"
#include "tasktimer/tasktimer.h"
#include "adc/adc.h"
#include "uart/uart.h"
#include "can/can.h"

#include "vehicleInterface/config/deviceMapping.h"
#include "vehicleInterface/config/configData.h"
#include "vehicleInterface/config/configDataDefault.h"
#include "vehicleInterface/vehicleState/vehicleState.h"
#include "vehicleInterface/vehicleControl/vehicleControl.h"

#include "device/gps/gps.h"
#include "device/sdc/sdc.h"
#include "device/pdm/pdm.h"
#include "device/pcinterface/pcinterface.h"
#include "device/inverter/cInverter.h"
#include "device/bms/bms.h"
#include "device/wheelspeed/wheelspeed.h"
#include "device/discretesense/discretesense.h"
#include "device/dashboard_output/dashboard_output.h"

#include "vehicleLogic/watchdogTrigger/watchdogTrigger.h"
#include "vehicleLogic/throttleController/throttleController.h"
#include "vehicleLogic/throttleController/torqueMap.h"
#include "vehicleLogic/stateManager/vehicleStateManager.h"


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
static Logging_T mLog = (Logging_T){};
static CRC_T mCrc = (CRC_T){
  .hcrc = &Mapping_CRC,
};
static ADC_Config_T mAdcConfig = (ADC_Config_T){
  .logger = &mLog,
  .handle = &Mapping_ADC,
  .adcIrq = Mapping_ADC_DMAStream,
  .numChannelsUsed = MAPPING_ADC_NUM_CHANNELS,
};

// Vehicle interface (1)
static VehicleState_T mVehicleState;

// ECU peripherals
static GPS_T mGps = (GPS_T){
  .pin3dFix = &Mapping_GPS_3dFixPin,
  .uart = MAPPING_GPS_UARTDEV,
  .state = &mVehicleState,
};
static PCInterface_T mPCInterface = (PCInterface_T){
  .uartA = MAPPING_PCINTERFACE_UARTADEV,
  .uartB = MAPPING_PCINTERFACE_UARTBDEV,
  .crc = &mCrc,
  .pinToggle = &Mapping_GPO_DebugToggle,
};
static PDM_T mPdm = (PDM_T){
  .channels = pdmChannels,
  .numChannels = PDM_NUM_CHANNELS,
  .vehicleState = &mVehicleState,
};
static SDC_Config_T mSdcConfig = (SDC_Config_T){
  .state = &mVehicleState,
  .pinBMS = &Mapping_GPI_SDC_BMS,
  .pinBSPD = &Mapping_GPI_SDC_BSPD,
  .pinIMD = &Mapping_GPI_SDC_IMD,
  .pinSDCOut = &Mapping_GPI_SDC_SDCOut,
  .pinECUError = &Mapping_GPO_SDC_ECUError,
};
static Wheelspeed_Config_T mWheelspeedConfig = {
  .logging = &mLog,
  .state = &mVehicleState,
  .frontWsPin = &Mapping_GPI_Wheelspeed_Front,
  .rearWsPin = &Mapping_GPI_Wheelspeed_Rear,
  .timerInstance = MAPPING_TIMER_2KHZ,
  // sensor teeth is applied after config is loaded in init
};

// Vehicle devices & sensors
static CInverter_T mInverter = (CInverter_T){
  .canInst = MAPPING_INVERTER_CANBUS,
  .vehicleState = &mVehicleState,
};
static BMS_T mBms = (BMS_T){
  .canInst = MAPPING_BMS_CANBUS,
  .vehicleState = &mVehicleState,
};
static DiscreteSense_T mDiscreteSense = (DiscreteSense_T){
  .logger = &mLog,
  .state = &mVehicleState,
  .adcAccelPedalA = MAPPING_ADC_THROTTLE_1,
  .adcAccelPedalB = MAPPING_ADC_THROTTLE_2,
  .adcBrakeFront = MAPPING_ADC_BRAKE_FRONT,
  .adcBrakeRear = MAPPING_ADC_BRAKE_REAR,
  .gpioDashboardButton = &Mapping_GPI_StartButton,
  // scaling is applied after config is loaded in init
};
static DashboardOut_T mDashboardOut = (DashboardOut_T){
  .output_pin = &Mapping_GPO_LED,
  .vehicleState = &mVehicleState,
  .log = &mLog,
};

// Vehicle interface (2)
static Config_T mConfig;
static VehicleControl_T mVehicleControl = (VehicleControl_T){
  .pdm = &mPdm,
  .inverter = &mInverter,
  .dashOut = &mDashboardOut,
};

// Vehicle processes
static WatchdogTrigger_T mWdtTrigger = (WatchdogTrigger_T){
  .blinkLED = &Mapping_GPO_LED,
};
static ThrottleController_T mThrottleController = (ThrottleController_T){
  .inputState = &mVehicleState,
  .control = &mVehicleControl,
  .vehicleConfig = &mConfig,
};
static VehicleStateManager_T mVehicleStateManager = (VehicleStateManager_T){
  .inputState = &mVehicleState,
  .control = &mVehicleControl,
  .throttleController = &mThrottleController,
  .vehicleConfig = &mConfig,
};

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

#define TRY_INIT(module_name, init_method, expected_result)     \
  if ((init_method) != (expected_result)) {                     \
    ECU_Init_Error("Watchdog Trigger initialization error\n");  \
  }

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
  TRY_INIT("Log", Log_Init(&mLog), LOGGING_STATUS_OK);

  Log_Print(&mLog, "###### ECU_Init_System ######\n");

  TRY_INIT("Task Timer", TaskTimer_Init(&mLog, &Mapping_Timer100Hz), TASKTIMER_STATUS_OK);
  // if (LOGGING_STATUS_OK != Log_EnableSWO(&mLog)) {
  //   ECU_Init_Error("Log_EnableSWO error\n");
  // }

  TRY_INIT("UART", UART_Init(&mLog), UART_STATUS_OK);
  TRY_INIT("USART1 (PCDebug A)", UART_Config(&Mapping_PCInterface_UARTA), UART_STATUS_OK);
  TRY_INIT("USART3 (PCDebug B)", UART_Config(&Mapping_PCInterface_UARTB), UART_STATUS_OK);
  TRY_INIT("CRC", CRC_Init(&mLog, &mCrc), CRC_STATUS_OK);

  // Create PC Debug driver this early so we get serial output, we'll enable the other
  // functionality later on
  TRY_INIT("PC Debug", PCInterface_Init(&mLog, &mPCInterface), PCINTERFACE_STATUS_OK);
}

//------------------------------------------------------------------------------
static void ECU_Init_BoardPeriph(void)
{
  Log_Print(&mLog, "###### ECU_Init_BoardPeriph ######\n");

  TRY_INIT("CAN", CAN_Init(&mLog), CAN_STATUS_OK);
  TRY_INIT("CAN1 bus", CAN_Config(CAN_DEV1, &Mapping_CAN1), CAN_STATUS_OK);
  TRY_INIT("CAN1 bus", CAN_Config(CAN_DEV2, &Mapping_CAN2), CAN_STATUS_OK);
  TRY_INIT("CAN1 bus", CAN_Config(CAN_DEV3, &Mapping_CAN3), CAN_STATUS_OK);
  mPCInterface.canDebugEnable = true;

  TRY_INIT("ADC", ADC_Init(&mAdcConfig), ADC_STATUS_OK);
}

//------------------------------------------------------------------------------
static void ECU_Init_BoardDevs(void)
{
  Log_Print(&mLog, "###### ECU_Init_BoardDevs ######\n");

  // TODO initialize EEPROM
}

//------------------------------------------------------------------------------
static void ECU_Init_LoadConfig(void)
{
  memset(&mConfig, 0, sizeof(mConfig));

  // TODO placeholder, update to load from EEPROM
  mConfig = DefaultConfigData;
}

//------------------------------------------------------------------------------
static void ECU_Init_VehicleInterface1(void)
{
  Log_Print(&mLog, "###### ECU_Init_VehicleInterface1 ######\n");

  TRY_INIT("Vehicle State", VehicleState_Init(&mLog, &mVehicleState), VEHICLESTATE_STATUS_OK);
  if (PCInterface_SetVehicleState(&mPCInterface, &mVehicleState) != PCINTERFACE_STATUS_OK) {
    ECU_Init_Error("PCInterface_SetVehicleState failed\n");
  }
}

//------------------------------------------------------------------------------
static void ECU_Init_VehicleDevices(void)
{
  Log_Print(&mLog, "###### ECU_Init_VehicleDevices ######\n");

  TRY_INIT("USART6 (GPS)", UART_Config(&Mapping_GPS_UART), UART_STATUS_OK);
  TRY_INIT("GPS", GPS_Init(&mLog, &mGps), GPS_STATUS_OK);

  mWheelspeedConfig.sensorTeeth = mConfig.inputs.numWheelspeedTeeth;
  TRY_INIT("Wheelspeed Sensor", Wheelspeed_Init(&mWheelspeedConfig), WHEELSPEED_STATUS_OK);
  TRY_INIT("Shutdown Circuit (SDC)", SDC_Init(&mLog, &mSdcConfig), SDC_STATUS_OK);
  TRY_INIT("Power Distribution Module (PDM)", PDM_Init(&mLog, &mPdm), PDM_STATUS_OK);

  // External devices
  TRY_INIT("Inverter", CInverter_Init(&mLog, &mInverter), CINVERTER_STATUS_OK);
  TRY_INIT("BMS", BMS_Init(&mLog, &mBms), BMS_STATUS_OK);

  // load discrete sensor settings from config
  mDiscreteSense.scalingAccelPedalA = (ADC_Scaling_T) {
    .lowerScaling = mConfig.inputs.accelPedal.calibrationA.rawLower,
    .upperScaling = mConfig.inputs.accelPedal.calibrationA.rawUpper,
    .saturate = true,
  };
  mDiscreteSense.scalingAccelPedalB = (ADC_Scaling_T) {
    .lowerScaling = mConfig.inputs.accelPedal.calibrationB.rawLower,
    .upperScaling = mConfig.inputs.accelPedal.calibrationB.rawUpper,
    .saturate = true,
  };
  mDiscreteSense.scalingBrakeFront = (ADC_Scaling_T) {
    .lowerScaling = mConfig.inputs.brakePressureFront.rawLower,
    .upperScaling = mConfig.inputs.brakePressureFront.rawUpper,
    .saturate = true,
  };
  mDiscreteSense.scalingBrakeRear = (ADC_Scaling_T) {
    .lowerScaling = mConfig.inputs.brakePressureRear.rawLower,
    .upperScaling = mConfig.inputs.brakePressureRear.rawUpper,
    .saturate = true,
  };
  TRY_INIT("Discrete Sensor", DiscreteSense_Init(&mDiscreteSense), DISCRETESENSE_STATUS_OK);
  TRY_INIT("Dashboard Out", DashboardOut_Init(&mDashboardOut), DASHBOARDOUT_OK);
}

//------------------------------------------------------------------------------
static void ECU_Init_VehicleInterface2(void)
{
  Log_Print(&mLog, "###### ECU_Init_VehicleInterface2 ######\n");

  TRY_INIT("Vehicle Control", VehicleControl_Init(&mLog, &mVehicleControl), VEHICLECONTROL_STATUS_OK);
  if (PCInterface_SetVehicleControl(&mPCInterface, &mVehicleControl) != PCINTERFACE_STATUS_OK) {
    ECU_Init_Error("PCInterface_SetVehicleControl error\n");
  }
}

//------------------------------------------------------------------------------
static void ECU_Init_VehicleProccesses(void)
{
  Log_Print(&mLog, "###### ECU_Init_VehicleProccesses ######\n");

  TRY_INIT("Watchdog Trigger", WatchdogTrigger_Init(&mLog, &mWdtTrigger), WATCHDOGTRIGGER_STATUS_OK);
  TRY_INIT("Throttle Controller", ThrottleController_Init(&mLog, &mThrottleController), THROTTLECONTROLLER_STATUS_OK);
  TRY_INIT("Vehicle State Manager", VehicleStateManager_Init(&mLog, &mVehicleStateManager), STATEMANAGER_STATUS_OK);
}
