/*
 * adc.h
 *
 *  Created on: 22 Nov 2020
 *      Author: Liam Flaherty
 */

#ifndef IO_ADC_ADC_H_
#define IO_ADC_ADC_H_

#include "stm32f7xx_hal.h"
#include <stdint.h>

#include "lib/logging/logging.h"

#define ADC_MAX_NUM_CHANNELS 16

typedef enum
{
  ADC_STATUS_OK                   = 0x00U,
  ADC_STATUS_ERROR_DMA            = 0x01U,
  ADC_STATUS_ERROR_CHANNEL_COUNT  = 0x02U
} ADC_Status_T;

typedef uint16_t ADC_Channel_T;

#define ADC_INVALID ((uint16_t)0xFFFFU)

/**
 * @brief Initialize ADC driver interface
 *
 * @param logger Pointer to logging settings
 * @param numChannelsUsed Number of channels in use
 * @param numSampleAvg Number of samples to average over.
 * For efficient averaging, this MUST be divisible by 2.
 *
 * @return Return status. ADC_STATUS_OK for success. See ADC_Status_T for more.
 */
ADC_Status_T ADC_Init(Logging_T* logger, const uint16_t numChannelsUsed, const uint16_t numSampleAvg);

/**
 * @brief Configure ADC device
 * This should be called from main. Main will retain ownership of handle ptr.
 * @param handle Handle for ADC device
 *
 * @return Return status. ADC_STATUS_OK for success. See ADC_Status_T for more.
 */
ADC_Status_T ADC_Config(ADC_HandleTypeDef* handle);

/**
 * @brief Gets the latest reading from the given ADC channel
 *
 * @returns Raw ADC reading (12-bit). ADC_INVALID (0xFFFF) if the channel is out of range.
 */
uint16_t ADC_Get(const ADC_Channel_T channel);


#endif /* IO_ADC_ADC_H_ */
