/*
 * ad5592r.h
 *
 *  Created on: 30 Apr 2021
 *      Author: Liam Flaherty
 */

#ifndef IO_AD5592R_AD5592R_H_
#define IO_AD5592R_AD5592R_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>

#define AD5592R_STACK_SIZE 200

typedef enum {
  AD5592R_STATUS_OK            = 0x00U,
  AD5592R_STATUS_SPI_ERROR     = 0x01U,
  AD5592R_STATUS_INVALID_VALUE = 0x02U,
  AD5592R_STATUS_INVALID_MODE  = 0x03U,
  AD5592R_STATUS_ERROR_SEM     = 0x04U,
  AD5592R_STATUS_ERROR_SPI     = 0x05U,
  AD5592R_STATUS_ERROR_TIME    = 0x06U
} AD5592R_Status_T;

typedef enum {
  AD5592R_IO0 = 0x0U,
  AD5592R_IO1 = 0x1U,
  AD5592R_IO2 = 0x2U,
  AD5592R_IO3 = 0x3U,
  AD5592R_IO4 = 0x4U,
  AD5592R_IO5 = 0x5U,
  AD5592R_IO6 = 0x6U,
  AD5592R_IO7 = 0x7U
} AD5592R_Channel_T;

typedef enum {
  AD5592R_MODE_NONE = 0x0U,
  AD5592R_MODE_DOUT = 0x1U,
  AD5592R_MODE_DIN  = 0x2U,
  AD5592R_MODE_AOUT = 0x3U,
  AD5592R_MODE_AIN  = 0x4U
} AD5592R_Mode_T;

typedef enum {
  AD5592R_PULLDOWN_DISABLED = 0x0U,
  AD5592R_PULLDOWN_ENABLED  = 0x1U
} AD5592R_Pulldown_T;

/**
 * Initialize the AD5592R device driver
 * The driver will sample each input and write each output at a periodic interval
 *
 * IMPORTANT: The SPI bus referred to by handle must be set up for 16 bit words
 *
 * @param handle SPI Device handle for SPI controller that AD5592R is connected to
 * @param csPinBank GPIO Pin Bank for SPI Chip Select pin
 * @param csPin SPI Chip Select pin
 */
AD5592R_Status_T AD5592R_Init(
    SPI_HandleTypeDef* handle,
    GPIO_TypeDef* csPinBank,
    uint16_t csPin);

/**
 * Configures a channel
 * @param channel Device channel to configure
 * @param mode Mode to set channel into
 */
AD5592R_Status_T AD5592R_ConfigChannel(
    const AD5592R_Channel_T channel,
    const AD5592R_Mode_T mode,
    const AD5592R_Pulldown_T pulldown);

/**
 * Returns the most recently sampled GPIO input on the specified channel
 * @param channel Channel to get
 * @param value Pointer to location to store sampled data
 * @return AD5592R_STATUS_OK if retrieval successful.
 */
AD5592R_Status_T AD5592R_DINGet(const AD5592R_Channel_T channel, uint16_t* value);

/**
 * Sets GPIO output on the specified channel to be written on the next conversion
 * @param channel Channel to set
 * @param value Value to set output to. Note that only 0 or 1 are valid values
 * @return AD5592R_STATUS_OK if successful.
 */
AD5592R_Status_T AD5592R_DOUTSet(const AD5592R_Channel_T channel, const uint16_t value);

/**
 * Returns the most recently sampled ADC input on the specified channel
 * @param channel Channel to get
 * @param value Pointer to location to store sampled data
 * @return AD5592R_STATUS_OK if retrieval successful.
 */
AD5592R_Status_T AD5592R_AINGet(const AD5592R_Channel_T channel, uint16_t* value);

/**
 * Sets DAC output on the specified channel to be written on the next conversion
 * @param channel Channel to set
 * @param value Value to set output to. Note that the device only uses 12-bits.
 * @return AD5592R_STATUS_OK if successful.
 */
AD5592R_Status_T AD5592R_AOUTSet(const AD5592R_Channel_T channel, const uint16_t value);

#endif /* IO_AD5592R_AD5592R_H_ */
