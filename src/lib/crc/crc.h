/*
 * crc.h
 *
 * Provides mutual exclusion for CRC hardware
 * 
 *  Created on: 20 Oct 2022
 *      Author: Liam Flaherty
 */

#ifndef LIB_CRC_CRC_H_
#define LIB_CRC_CRC_H_

#include "stm32f7xx_hal.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdint.h>
#include <stdbool.h>

#include "lib/logging/logging.h"

typedef enum
{
  CRC_STATUS_OK     = 0x00U,
  CRC_STATUS_ERROR  = 0x01U,
} CRC_Status_T;

typedef struct
{
  CRC_HandleTypeDef* hcrc; // CRC calculation hardware

  // ******* Internal use *******
  // Mutex lock
  SemaphoreHandle_t mutex;
  StaticSemaphore_t mutexBuffer;
} CRC_T;

/**
 * @brief Initialize CRC wrapper
 * @param logger Pointer to logging module
 */
CRC_Status_T CRC_Init(Logging_T* logger, CRC_T* crcObj);

/**
 * @brief Attempts to perform a CRC calculation. Will provide mutual exclusion.
 * This is a blocking method, and will pend until the hardware is free, or
 * timeout is reached.
 * 
 * @param crcObj CRC module
 * @param buffer Input data to calculate CRC for.
 * @param bufferLen Length for buffer.
 * @param timeout Ticks for mutex pend timeout.
 * @param crcOut This will be updated to the CRC value, or set to 0 if CRC
 * calculation is unsuccessful.
 * @return true if CRC was calculated.
 */
bool CRC_Calculate(
  CRC_T* crcObj,
  uint32_t buffer[],
  uint32_t bufferLen,
  TickType_t timeout,
  uint32_t* crcOut);

#endif /* LIB_CRC_CRC_H_ */
