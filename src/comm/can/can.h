/*
 * can.h
 *
 *  Created on: Oct 22, 2020
 *      Author: Liam Flaherty
 */

#ifndef COMM_CAN_CAN_H_
#define COMM_CAN_CAN_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>

#include "lib/logging/logging.h"

#define CAN_NUM_INSTANCES 3

typedef enum
{
  CAN_DEV1 = 0x00U,
  CAN_DEV2 = 0x01U,
  CAN_DEV3 = 0x02U
} CAN_Device_T;

#define CAN_MAX_RECV_QUEUES 8  // queues per CAN bus instance
#define CAN_MAX_PENDING_MSGS 64 // messages that can pend at once

typedef enum
{
  CAN_STATUS_OK		               = 0x00U,
  CAN_STATUS_ERROR_TX            = 0x01U,
  CAN_STATUS_ERROR_CFG_FILTER    = 0x02U,
  CAN_STATUS_ERROR_START         = 0x03U,
  CAN_STATUS_ERROR_START_NOTIFY  = 0x04U,
  CAN_STATUS_ERROR_INVALID_BUS   = 0x05U,
  CAN_STATUS_ERROR_MAX_QUEUES    = 0x06U
} CAN_Status_T;

/**
 * @brief Data structure used to store CAN frames
 */
typedef struct {
  CAN_Device_T busInstance;
  uint32_t msgId;
  uint8_t data[8];
  uint32_t dlc;
} CAN_DataFrame_T;

/**
 * @brief Initialize CAN driver interface
 */
CAN_Status_T CAN_Init(Logging_T* logger);

/**
 * @brief Configure CAN bus
 *
 * @param device Name of CAN instance
 * @param handle STM HAL handle for device
 * @return Return status. CAN_STATUS_OK for success. See CAN_Status_T for more.
 */
CAN_Status_T CAN_Config(CAN_Device_T device, CAN_HandleTypeDef* handle);

/**
 * @brief Adds a queue to send data to.
 * Will send data to this queue if (msg id & deviceIdMask) == deviceId
 * 
 * E.g.:
 * A message ID may be: 0x4A1, where the 0x4 signifies the device, and the
 * 0xA1 signifies a message ID from that device.
 * To capture this, use:
 *    deviceId      = 0x400
 *    deviceIdMask  = 0xF00
 * The deviceIdMask should be selected that it also covers the number of bits
 * used by other devices on the bus.
 * 
 * @param canInstance CAN Bus device instance
 * @param deviceId ID of device with zero offset.
 * @param deviceIdMask Mask that will cause msg id to match device id when applied.
 * @param outQueue Queue to send data to
 * @return CAN_STATUS_OK if successful.
 */
CAN_Status_T CAN_RegisterQueue(
    const CAN_Device_T canInstance,
    const uint32_t deviceId,
    const uint32_t deviceIdMask,
    QueueHandle_t outQueue);

/**
 * @brief Send a message on the CAN bus
 *
 * @param canInstance CAN Bus device instance
 * @param msgId CAN Frame ID
 * @param data Array of data to send
 * @param n Length of data array. Max 8.
 * @return Return status. CAN_STATUS_OK for success. See CAN_Status_T for more.
 * handle->ErrorCode may provide more detailed error information.
 */
CAN_Status_T CAN_SendMessage(
    const CAN_Device_T canInstance,
    uint32_t msgId,
    uint8_t* data,
    uint32_t n);

#endif /* COMM_CAN_CAN_H_ */
