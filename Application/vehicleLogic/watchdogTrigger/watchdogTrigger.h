/*
 * watchdogTrigger.h
 *
 *  Created on: Jul 31 2022
 *      Author: Liam Flaherty
 */

#ifndef VEHICLELOGIC_WATCHDOGTRIGGER_WATCHDOGTRIGGER_H_
#define VEHICLELOGIC_WATCHDOGTRIGGER_WATCHDOGTRIGGER_H_

#include "lib/depends/depends.h"
#include "lib/logging/logging.h"

#include "io/gpio/gpio.h"

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

typedef enum
{
  WATCHDOGTRIGGER_STATUS_OK             = 0x00U,
  WATCHDOGTRIGGER_STATUS_ERROR_INIT     = 0x01U,
  WATCHDOGTRIGGER_STATUS_ERROR_DEPENDS  = 0x02U,
} WatchdogTrigger_Status_T;

#define WATCHDOGTRIGGER_STACK_SIZE 2000U
#define WATCHDOGTRIGGER_TASK_PRIORITY 8U

typedef struct {
  // Config
  GPIO_T* blinkLED;

  // ******* Internal use *******
  uint16_t counter;

  // Mutex lock
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;

  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[WATCHDOGTRIGGER_STACK_SIZE];

  REGISTERED_MODULE();
} WatchdogTrigger_T;

/**
 * @brief Initialize the watchdog timer trigger.
 * 
 * @param logger Pointer to system logger
 * @param wdtTrigger WatchdogTrigger struct
 * @return WATCHDOGTRIGGER_STATUS_OK if init successful 
 */
WatchdogTrigger_Status_T WatchdogTrigger_Init(
    Logging_T* logger,
    WatchdogTrigger_T* wdtTrigger);

#endif // VEHICLELOGIC_WATCHDOGTRIGGER_WATCHDOGTRIGGER_H_
