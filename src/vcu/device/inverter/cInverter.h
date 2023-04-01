/*
 * cInverter.h
 *
 *  Created on: Oct 7 2021
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_INVERTER_CINVERTER_H_
#define DEVICE_INVERTER_CINVERTER_H_

#include "lib/depends/depends.h"
#include "lib/logging/logging.h"

#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"

#include "comm/can/can.h"
#include "vehicleInterface/vehicleState/vehicleState.h"

#include "cInverterCAN.h"  /* Defines offset CAN IDs */


#define INVERTER_STACK_SIZE 2000
#define INVERTER_TASK_PRIORITY 10
#define INVERTER_QUEUE_LENGTH 256
#define INVERTER_QUEUE_DATA_SIZE sizeof(CAN_DataFrame_T)

#define INVERTER_CAN_DEVICEID     0
#define INVERTER_CAN_DEVICEIDMASK 0xF00

// Maximum supported torque value in message format
#define INVERTER_MAX_TORQUE 3276.7f

struct CInverterCommand {
  bool inverterEnabled;
  bool dischargeModeEnabled;

  // Mutex lock for commanding state
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;
};

typedef struct
{
  // ******* Setup *******
  CAN_Device_T canInst;       // CAN device connected to inverter
  VehicleState_T* vehicleState;

  // ******* Internal use *******
  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[VEHICLESTATE_STACK_SIZE];

  // Queue
  QueueHandle_t canDataQueueHandle;
  StaticQueue_t canDataQueueBuffer;
  uint8_t canDataQueueStorageArea[INVERTER_QUEUE_LENGTH*INVERTER_QUEUE_DATA_SIZE];

  // Commanding state:
  struct CInverterCommand commandData;

  REGISTERED_MODULE();
} CInverter_T;

typedef enum
{
  CINVERTER_STATUS_OK                   = 0x00U,
  CINVERTER_STATUS_ERROR_INIT           = 0x01U,
  CINVERTER_STATUS_ERROR_CAN            = 0x02U,
  CINVERTER_STATUS_ERROR_DEPENDS        = 0x03U,
  CINVERTER_STATUS_ERROR_LOCK           = 0x04U,
  CINVERTER_STATUS_ERROR_NOT_ENABLED    = 0x05U,
  CINVERTER_STATUS_ERROR_NOT_DISIABLED  = 0x06U,
  CINVERTER_STATUS_ERROR_DISCHARGE      = 0x07U,
  CINVERTER_STATUS_ERROR_VALUE          = 0x08U,
} CInverter_Status_T;

/**
 * @brief Initialize the inverter driver
 * @param logger Pointer to logging settings
 * @param inv inverter data
 */
CInverter_Status_T CInverter_Init(Logging_T* logger, CInverter_T* inv);

/**
 * @brief Send a normal torque command to instruct inverter to power to the
 * required value of Nm.
 * CInverter_SendInverterEnabled must be used to enable the inverter before
 * torque can be requested.
 * 
 * @param inv Inverter module
 * @param torqueNm Torque to motor to [Nm]
 * @param direction Direction of motion
 * @return CInverter_Status_T CINVERTER_STATUS_OK if request sent successfully
 */
CInverter_Status_T CInverter_SendTorqueCommand(CInverter_T* inv, const float torqueNm, const VehicleState_InverterDirection_T direction);

/**
 * @brief Enable or disable the inverter power output.
 * 
 * @param inv Inverter module
 * @param enabled Enable or disable inverter power output.
 * @return CInverter_Status_T CINVERTER_STATUS_OK if request sent successfully
 */
CInverter_Status_T CInverter_SendInverterEnabled(CInverter_T* inv, const bool enabled);

/**
 * @brief Enable or disable discharge mode in inverter.
 * Enabling discharge mode will disable power output.
 * 
 * @param inv Inverter module
 * @param dischargeModeEnabled Enable or disbale discharge mode.
 * @return CInverter_Status_T CINVERTER_STATUS_OK if request sent successfully
 */
CInverter_Status_T CInverter_SendInverterDischarge(CInverter_T* inv, const bool dischargeModeEnabled);

#endif /* DEVICE_INVERTER_CINVERTER_H_ */
