/*
 * gps.c
 *
 *  Created on: 22 Oct 2022
 *      Author: Liam Flaherty
 */
#include "gps.h"

#include <stdio.h> // for snprintf
#include <string.h>
#include "comm/uart/uart.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms

// ------------------- Private methods -------------------
/**
 * @brief Push the contents of data into state.
 * 
 * @param state 
 * @param data 
 */
static void pushGPGGA(VehicleState_T* state, struct NmeaMessageGPGGA* data)
{
  if (VehicleState_AccessAcquire(state)) {
    state->data.vehicle.gps.utcTime = data->utcTime;
    state->data.vehicle.gps.nsIndicator = data->nsIndicator;
    state->data.vehicle.gps.ewIndicator = data->ewIndicator;
    state->data.vehicle.gps.positionFix = data->positionFix;
    state->data.vehicle.gps.nSatellites = data->nSatellites;
    memcpy(state->data.vehicle.gps.latitude, data->latitude, sizeof(data->latitude) * sizeof(char));
    memcpy(state->data.vehicle.gps.longitude, data->longitude, sizeof(data->longitude) * sizeof(char));
  }

  VehicleState_AccessRelease(state);
}

static void GPS_TaskMethod(GPS_T* gps)
{
  size_t receivedBytes = xStreamBufferReceive(
      gps->recvStreamHandle,
      gps->workingBuffer,
      GPS_RECV_STREAM_SIZE_BYTES,
      mBlockTime);
  
  if (0U == receivedBytes) {
    return;
  }

  // Log these messages
  // TODO remove this after hardware testing
  char buff[2*GPS_RECV_STREAM_SIZE_BYTES] = {0};
  int offset = snprintf(buff, GPS_RECV_STREAM_SIZE_BYTES, "GPS received sentence: ");
  for (int i = 0; i < (int)receivedBytes; ++i) {
    offset += snprintf(buff + offset, GPS_RECV_STREAM_SIZE_BYTES, "%c", gps->workingBuffer[i]);
  }
  offset += snprintf(buff + offset, GPS_RECV_STREAM_SIZE_BYTES, "\n");

  // Handle the message
  bool accumulateSucc = NmeaDecode_AccumulateBytes(
      &gps->nmeaDecoder,
      gps->workingBuffer,
      (uint16_t)receivedBytes);
  if (!accumulateSucc) {
    return;
  }

  // Push data to vehicle state
  NmeaMessageFields_T decodedFields;
  while (NmeaDecode_Decode(&gps->nmeaDecoder, &decodedFields)) {
    switch (decodedFields.type) {
      case NMEA_MSGTYPE_NULL:
        break;

      case NMEA_MSGTYPE_GPGGA:
        pushGPGGA(gps->state, &decodedFields.fields.gpgga);
        break;
      
      case NMEA_MSGTYPE_GPRMC:
        // TODO
        break;
      
      case NMEA_MSGTYPE_GPVTG:
        // TODO
        break;
    }
  }
}

static void GPS_Task(void* pvParameters)
{
  GPS_T* obj = (GPS_T*)pvParameters;

  while (1) {
    GPS_TaskMethod(obj);
  }
}

// ------------------- Public methods -------------------
GPS_Status_T GPS_Init(
    Logging_T* logger,
    GPS_T* gps)
{
  mLog = logger;
  Log_Print(mLog, "GPS_Init begin\n");
  DEPEND_ON(logger, GPS_STATUS_ERROR_DEPENDS);
  DEPEND_ON(gps->state, GPS_STATUS_ERROR_DEPENDS);
  DEPEND_ON_STATIC(UART, GPS_STATUS_ERROR_DEPENDS);

  if (NULL == gps->pin3dFix || NULL == gps->state) {
    return GPS_STATUS_ERROR_INIT;
  }

  NmeaDecode_Init(&gps->nmeaDecoder);

  // create main task
  gps->taskHandle = xTaskCreateStatic(
      GPS_Task,
      "GPS",
      GPS_STACK_SIZE,
      (void*)gps,  /* Parameter passed as pointer */
      GPS_TASK_PRIORITY,
      gps->taskStack,
      &gps->taskBuffer);

  // Register to receive serial data
  gps->recvStreamHandle = xStreamBufferCreateStatic(
      GPS_RECV_STREAM_SIZE_BYTES,
      GPS_RECV_STREAM_TRIGGER_LEVEL_BYTES,
      gps->recvStreamStorage,
      &gps->recvStreamStruct);
  if (UART_STATUS_OK != UART_SetRecvStream(gps->uart, gps->recvStreamHandle)) {
    return GPS_STATUS_ERROR_INIT;
  }

  REGISTER(gps, GPS_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "GPS_Init complete\n");
  return GPS_STATUS_OK;
}
