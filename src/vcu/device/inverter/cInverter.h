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

  REGISTERED_MODULE();
} CInverter_T;

typedef enum
{
  CINVERTER_STATUS_OK             = 0x00U,
  CINVERTER_STATUS_ERROR_CAN      = 0x01U,
  CINVERTER_STATUS_ERROR_DEPENDS  = 0x02U,
} CInverter_Status_T;

/**
 * @brief Initialize the inverter driver
 * @param logger Pointer to logging settings
 * @param inv inverter data
 */
CInverter_Status_T CInverter_Init(Logging_T* logger, CInverter_T* inv);

// TODO add inverter control


#endif /* DEVICE_INVERTER_CINVERTER_H_ */
