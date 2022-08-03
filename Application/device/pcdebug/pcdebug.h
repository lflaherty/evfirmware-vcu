/*
 * pcdebug.h
 *
 *  Created on: Aug 2 2022
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_PCDEBUG_PCDEBUG_H_
#define DEVICE_PCDEBUG_PCDEBUG_H_

#include "lib/logging/logging.h"

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

typedef enum
{
  PCDEBUG_STATUS_OK         = 0x00,
  PCDEBUG_STATUS_ERROR_INIT = 0x01
} PCDebug_Status_T;

#define PCDEBUG_STACK_SIZE 2000U
#define PCDEBUG_TASK_PRIORITY 3U

typedef struct
{
  // Config
  // TODO UART object

  // ******* Internal use *******
  uint16_t counter;

  // Mutex lock
  // TODO assess whether this is used
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;

  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[PCDEBUG_STACK_SIZE];
} PCDebug_T;

/**
 * @brief Initialize the PC Debug interface.
 * 
 * @param logger Pointer to system logger.
 * @param pcdebug PCDebug struct
 * @return PCDEBUG_STATUS_OK if init successful 
 */
PCDebug_Status_T PCDebug_Init(
    Logging_T* logger,
    PCDebug_T* pcdebug);

#endif // DEVICE_PCDEBUG_PCDEBUG_H_