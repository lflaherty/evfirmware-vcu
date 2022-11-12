/*
 * gps.h
 *
 * Driver for onboard GPS receiver.
 * 
 *  Created on: 22 Oct 2022
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_GPS_GPS_H_
#define DEVICE_GPS_GPS_H_

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"

#include "lib/depends/depends.h"
#include "lib/logging/logging.h"
#include "io/gpio/gpio.h"
#include "comm/uart/nmeadecode.h"

#include "vehicleInterface/vehicleState/vehicleState.h"

typedef enum
{
  GPS_STATUS_OK             = 0x00,
  GPS_STATUS_ERROR_INIT     = 0x01,
  GPS_STATUS_ERROR_DEPENDS  = 0x02,
} GPS_Status_T;

#define GPS_STACK_SIZE 2000U
#define GPS_TASK_PRIORITY 4U

#define GPS_RECV_STREAM_SIZE_BYTES 2048U
#define GPS_RECV_STREAM_TRIGGER_LEVEL_BYTES 1U

typedef struct
{
  UART_HandleTypeDef* huart; // UART handle to receive GPS data from
  GPIO_T* pin3dFix; // GPIO pin for 3D fix input
  VehicleState_T* state; // Vehicle state object to push data to

  // ******* Internal use *******
  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[GPS_STACK_SIZE];

  // Stream buffer objects for receiving uart bytes
  uint8_t recvStreamStorage[GPS_RECV_STREAM_SIZE_BYTES];
  StaticStreamBuffer_t recvStreamStruct;
  StreamBufferHandle_t recvStreamHandle;

  // NMEA sentence decoder
  NmeaDecode_T nmeaDecoder;

  // For taking data out of serial stream
  uint8_t workingBuffer[GPS_RECV_STREAM_SIZE_BYTES];

  REGISTERED_MODULE();
} GPS_T;

/**
 * @brief Initialize GPS driver
 * Starts RTOS task and assigns UART receive handler
 * 
 * @param logger Pointer to system logger
 * @param gps GPS struct
 * @return GPS_STATUS_OK if init successful 
 */
GPS_Status_T GPS_Init(
    Logging_T* logger,
    GPS_T* gps);

#endif // DEVICE_GPS_GPS_H_
