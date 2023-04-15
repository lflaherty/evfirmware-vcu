/*
 * bms.h
 *
 *  Created on: 15 Apr 2023
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_BMS_BMS_H_
#define DEVICE_BMS_BMS_H_

#include "lib/depends/depends.h"
#include "lib/logging/logging.h"

#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"

#include "comm/can/can.h"
#include "vehicleInterface/vehicleState/vehicleState.h"


#define BMS_STACK_SIZE 2000
#define BMS_TASK_PRIORITY 10
#define BMS_QUEUE_LENGTH 256
#define BMS_QUEUE_DATA_SIZE sizeof(CAN_DataFrame_T)

typedef struct
{
  // ******* Setup *******
  CAN_Device_T canInst;       // CAN device connected to BMS
  VehicleState_T* vehicleState; // Module to push data to

  // ******* Internal use *******
  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[VEHICLESTATE_STACK_SIZE];

  // Queue
  QueueHandle_t canDataQueueHandle;
  StaticQueue_t canDataQueueBuffer;
  uint8_t canDataQueueStorageArea[BMS_QUEUE_LENGTH*BMS_QUEUE_DATA_SIZE];

  REGISTERED_MODULE();
} BMS_T;

typedef enum
{
  BMS_STATUS_OK                   = 0x00U,
  BMS_STATUS_ERROR_INIT           = 0x01U,
  BMS_STATUS_ERROR_CAN            = 0x02U,
  BMS_STATUS_ERROR_DEPENDS        = 0x03U,
  BMS_STATUS_ERROR_LOCK           = 0x04U,
  BMS_STATUS_ERROR_NOT_ENABLED    = 0x05U,
} BMS_Status_T;

/**
 * @brief Initialize the inverter driver
 * @param logger Pointer to logging settings
 * @param inv inverter data
 */
BMS_Status_T BMS_Init(Logging_T* logger, BMS_T* inv);


#endif /* DEVICE_BMS_BMS_H_ */
