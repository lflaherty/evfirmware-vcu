/*
 * adc.h
 *
 * ADC interface abstraction.
 *
 *  Created on: 22 Nov 2020
 *      Author: Liam Flaherty
 */

#ifndef IO_ADC_ADC_H_
#define IO_ADC_ADC_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

#include "depends/depends.h"
#include "logging/logging.h"

REGISTERED_MODULE_STATIC(ADC);

typedef enum
{
  ADC_STATUS_OK                     = 0x00,
  ADC_STATUS_ERROR_DMA              = 0x01,
  ADC_STATUS_ERROR_CHANNEL_COUNT    = 0x02,
  ADC_STATUS_ERROR_DEPENDS          = 0x03,
  ADC_STATUS_ERROR_HW_CONFIG        = 0x04,
  ADC_STATUS_ERROR_INVALID_CHANNEL  = 0x05,
  ADC_STATUS_ERROR_INTERNAL         = 0x07,
  ADC_STATUS_ERROR_DATANOTREADY     = 0x08,
} ADC_Status_T;

typedef enum {
  ADC_CONVERSIONCHANNEL0 = 0,
  ADC_CONVERSIONCHANNEL1,
  ADC_CONVERSIONCHANNEL2,
  ADC_CONVERSIONCHANNEL3,
  ADC_CONVERSIONCHANNEL4,
  ADC_CONVERSIONCHANNEL5,
  ADC_CONVERSIONCHANNEL6,
  ADC_CONVERSIONCHANNEL7,
  ADC_CONVERSIONCHANNEL8,
  ADC_CONVERSIONCHANNEL9,
  ADC_CONVERSIONCHANNEL10,
  ADC_CONVERSIONCHANNEL11,
  ADC_CONVERSIONCHANNEL12,
  ADC_CONVERSIONCHANNEL13,
  ADC_CONVERSIONCHANNEL14,
  ADC_CONVERSIONCHANNEL15,
  ADC_MAX_NUM_CHANNELS
} ADC_Channel_T;
_Static_assert( (ADC_MAX_NUM_CHANNELS / 2) * 2 == ADC_MAX_NUM_CHANNELS, "Value must be multiple of 2");


/**
 * @brief Used to read from a channel and apply scaling to obtain a fraction
 * between the lower and upper values.
 */
typedef struct
{
  uint16_t lowerScaling;
  uint16_t upperScaling;
  bool saturate;
} ADC_Scaling_T;

typedef struct
{
  Logging_T* logger;
  ADC_HandleTypeDef* handle;
  IRQn_Type adcIrq; // The IRQ for this ADC

  uint16_t numChannelsUsed; // Must be consistent with ADC hardware init settings.

  // TODO add oversampling
} ADC_Config_T;

/**
 * @brief Initialize ADC driver interface
 *
 * @param config ADC config settings.
 *
 * @return Return status. ADC_STATUS_OK for success. See ADC_Status_T for more.
 */
ADC_Status_T ADC_Init(ADC_Config_T* config);

/**
 * @brief Gets the latest reading from the given ADC channel
 *
 * @param channel ADC channel, 0 up to the value in ADC_Config_T::numChannelsUsed
 * @param val Output raw ADC reading (12-bit). 0 if the channel is out of range.
 * @returns Return status. ADC_STATUS_OK for success. See ADC_Status_T for more.
 */
ADC_Status_T ADC_Get(const ADC_Channel_T channel, uint16_t* val);

/**
 * @brief Applies a linear scaling between the lower and upper values,
 * resulting in a scaled value between [0,1].
 * 
 * If channel->saturate == true, then val won't exceed the [0,1] limits.
 * 
 * @param scaling Scaling settings to use.
 * @param raw A raw ADC reading.
 * @return Output scaled value.
 */
float ADC_ApplyScaling(
    const ADC_Scaling_T* scaling,
    const uint16_t raw);


#endif /* IO_ADC_ADC_H_ */
