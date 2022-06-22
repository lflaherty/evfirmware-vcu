/*
 * throttleController.h
 *
 *  Created on: Jun 19 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLELOGIC_THROTTLECONTROLLER_THROTTLECONTROLLER_H_
#define VEHICLELOGIC_THROTTLECONTROLLER_THROTTLECONTROLLER_H_

#include <stdint.h>
#include <stdbool.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "lib/logging/logging.h"

#include "vehicleInterface/vehicleState/vehicleState.h"
#include "vehicleInterface/vehicleControl/vehicleControl.h"
#include "vehicleInterface/config/configData.h"

#include "torqueMap.h"

typedef enum
{
  THROTTLECONTROLLER_STATUS_OK          = 0x00U,
  THROTTLECONTROLLER_STATUS_ERROR_INIT  = 0x01U,
  THROTTLECONTROLLER_STATUS_ERROR_MUTEX = 0x02U
} ThrottleController_Status_T;

#define THROTTLECONTROLLER_STACK_SIZE 2000
#define THROTTLECONTROLLER_TASK_PRIORITY 3

typedef struct
{
  // Config
  VehicleState_T* inputState; // must be set prior to initialization
  VehicleControl_T* control; // must be set prior to initialization
  Config_T* vehicleConfig; // must be set prior to initialization

  // ******* Internal use *******
  TorqueMap_T* torqueMapForward;
  TorqueMap_T* torqueMapReverse;
  VehicleState_InverterDirection_T direction;
  bool enabled;

  // Mutex lock
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;

  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[THROTTLECONTROLLER_STACK_SIZE];
} ThrottleController_T;

/**
 * @brief Initialize the throttle controller
 * 
 * @param logger Pointer to system logger
 * @param throttleControl Pointer to ThrottleController object
 * @return Success status. THROTTLECONTROLLER_STATUS_OK if ok.
 */
ThrottleController_Status_T ThrottleController_Init(
    Logging_T* logger,
    ThrottleController_T* throttleControl);

/**
 * @brief Enables or disables motor motion.
 * Note, disabled will just send a 0Nm torque request.
 * 0Nm torque commands are still made to continue communication with inverter.
 * 
 * @param throttleControl ThrottleController object
 * @param enabled Whether to output torque.
 * @return THROTTLECONTROLLER_STATUS_OK if successful. 
 */
ThrottleController_Status_T ThrottleController_SetTorqueEnabled(
    ThrottleController_T* throttleControl,
    bool enabled);

/**
 * @brief Sets direction for motor motion.
 * 
 * @param throttleControl ThrottleController object
 * @param direction Commanded motor direction.
 * @return THROTTLECONTROLLER_STATUS_OK if successful. 
 */
ThrottleController_Status_T ThrottleController_SetMotorDirection(
    ThrottleController_T* throttleControl,
    VehicleState_InverterDirection_T direction);

#endif // VEHICLELOGIC_THROTTLECONTROLLER_THROTTLECONTROLLER_H_