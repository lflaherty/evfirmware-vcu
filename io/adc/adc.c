/*
 * adc.c
 *
 *  Created on: 22 Nov 2020
 *      Author: Liam Flaherty
 */

#include "adc.h"

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "lib/logging/logging.h"


static Logging_T* logging;

/**
 * Used by DMA to store data
 */
static volatile uint16_t adcDMABuf[ADC_NUM_CHANNELS];

/**
 * Used to store intermediate ADC counting values.
 * (The sum value before averaging)
 */
static volatile uint16_t adcDataCounting[ADC_NUM_CHANNELS];

/**
 * Used to store final ADC results
 */
static volatile uint16_t adcData[ADC_NUM_CHANNELS];

/**
 * Number of samples to average over
 */
static uint16_t numSamples;

/**
 * Use this in a logical shift >> as a cheap divide.
 * Do this to avoid the 1/n divide required in the average calc.
 */
static uint16_t averagingShift;

/**
 * How many counts have occurred.
 */
static uint16_t currentSampleCount;

static bool adcInitialized = false;


// ------------------- Public methods -------------------
ADC_Status_T ADC_Init(Logging_T* logger, const uint16_t numSampleAvg)
{
  logging = logger;
  logPrintS(logging, "ADC_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // Initialize buffers to 0 (can't use memset due to volatile)
  for (size_t i = 0; i < ADC_NUM_CHANNELS; ++i) {
    adcDMABuf[i] = 0;
    adcData[i] = 0;
    adcDataCounting[i] = 0;
  }

  numSamples = numSampleAvg;
  averagingShift = log(numSamples) / log(2);
  currentSampleCount = 0;

  adcInitialized = true;

  logPrintS(logging, "ADC_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return ADC_STATUS_OK;
}

//------------------------------------------------------------------------------
ADC_Status_T ADC_Config(ADC_HandleTypeDef* handle)
{
  logPrintS(logging, "ADC_Config begin\n", LOGGING_DEFAULT_BUFF_LEN);
  // TODO: this only works for ADC1 (with multiple channels), expand to ADCx

  // just need to start DMA
  if (HAL_ADC_Start_DMA(handle, (uint32_t*)adcDMABuf, ADC_NUM_CHANNELS) != HAL_OK) {
    return ADC_STATUS_ERROR_DMA;
  }

  logPrintS(logging, "ADC_Config complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return ADC_STATUS_OK;
}

//------------------------------------------------------------------------------
uint16_t ADC_Get(const ADC_Channel_T channel)
{
  return adcData[channel];
}


// ------------------- Interrupts -------------------
/**
 * @brief Called when first half of buffer is filled
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  // Empty
}

/**
 * @brief Called when buffer is completely filled
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if (adcInitialized) {

    ++currentSampleCount;

    if (currentSampleCount == numSamples) {
      // copy results into averaging buffer
      for (size_t i = 0; i < ADC_NUM_CHANNELS; ++i) {
        adcDataCounting[i] += adcDMABuf[i] & 0xFFF;  // 12 bit, it should be this anyway, but make sure
      }

      // reset counter
      currentSampleCount = 0;
    } else {
      // copy results into final buffer
      for (size_t i = 0; i < ADC_NUM_CHANNELS; ++i) {
        adcData[i] = adcDataCounting[i] >> averagingShift; // use right shift as a cheap divide
        adcDataCounting[i] = 0;
      }
    }

  }
}
