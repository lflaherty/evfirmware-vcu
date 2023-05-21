/*
 * adc.c
 *
 *  Created on: 22 Nov 2020
 *      Author: Liam Flaherty
 */

#include "adc.h"

#include "stm32f7xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>

#include "lib/depends/depends.h"
#include "lib/logging/logging.h"

REGISTERED_MODULE_STATIC_DEF(ADC);

static Logging_T* logging;

static uint32_t adcDMABuf[ADC_MAX_NUM_CHANNELS]; // DMA buffer
static volatile uint32_t copyBufferA[ADC_MAX_NUM_CHANNELS];
static volatile uint32_t copyBufferB[ADC_MAX_NUM_CHANNELS];

// The "active" buffer is the buffer that will be copied from DMA next
static volatile enum {
  COPY_BUFFER_A_ACTIVE,
  COPY_BUFFER_B_ACTIVE
} activeCopyBuffer; 

// details for copying to buffers
size_t halfCpltCopyLen; // Number of elements to copy at ADC DMA half complete
size_t cpltCopyOffset; // Array offset to copy from at ADC DMA full complete
size_t cpltCopyLen; // Number of elements to copy at ADC DMA full complete

static uint16_t numChannels; // Number of ADC channels in use

static ADC_HandleTypeDef* adcHandle;
static IRQn_Type adcIrq;

static bool adcInitialized = false;

// ------------------- Private methods -------------------
/**
 * @brief Alternative to memcpy that works on volatile uint32_t arrays
 * 
 * @param dest 
 * @param src 
 * @param len 
 */
void bufferCopy(volatile uint32_t* dest, volatile uint32_t* src, size_t len)
{
  for (size_t i = 0; i < len; ++i) {
    dest[i] = src[i];
  }
}

/**
 * @brief Alternative to memset to operate on volatile uint32_t arrays
 * 
 * @param dest 
 * @param val 
 * @param len 
 */
void bufferSet(volatile uint32_t* dest, volatile uint32_t val, size_t len)
{
  for (size_t i = 0; i < len; ++i) {
    dest[i] = val;
  }
}

// ------------------- Public methods -------------------
ADC_Status_T ADC_Init(ADC_Config_T* config)
{
  logging = config->logger;
  Log_Print(logging, "ADC_Init begin\n");
  DEPEND_ON(logging, ADC_STATUS_ERROR_DEPENDS);

  if (config->numChannelsUsed > ADC_MAX_NUM_CHANNELS) {
    return ADC_STATUS_ERROR_CHANNEL_COUNT;
  }

  ADC_InitTypeDef* hwInit = &config->handle->Init;
  if (config->numChannelsUsed != hwInit->NbrOfConversion) {
    Log_Print(logging, "ADC_Init number of channels does not match hardware settings\n");
    return ADC_STATUS_ERROR_HW_CONFIG;
  }
  if (ENABLE != hwInit->ContinuousConvMode) {
    Log_Print(logging, "ADC_Init hardware setting ContinuousConvMode required\n");
    return ADC_STATUS_ERROR_HW_CONFIG;
  }
  if (ADC_DATAALIGN_RIGHT != hwInit->DataAlign) {
    Log_Print(logging, "ADC_Init hardware setting DataAlign must be aligned right\n");
    return ADC_STATUS_ERROR_HW_CONFIG;
  }

  // Initialize buffers to 0 (can't use memset due to volatile)
  bufferSet(adcDMABuf, 0, ADC_MAX_NUM_CHANNELS);
  bufferSet(copyBufferA, 0, ADC_MAX_NUM_CHANNELS);
  bufferSet(copyBufferB, 0, ADC_MAX_NUM_CHANNELS);

  numChannels = config->numChannelsUsed;
  adcHandle = config->handle;
  adcIrq = config->adcIrq;

  // Copy buffer settings
  activeCopyBuffer = COPY_BUFFER_A_ACTIVE;
  // For the half complete event, in the event of an uneven number of data
  // points, just round down with integer divide
  halfCpltCopyLen = numChannels / 2;
  cpltCopyOffset = numChannels / 2;
  if (numChannels % 2 == 0) {
    cpltCopyLen = numChannels / 2;
  } else {
    cpltCopyLen = numChannels / 2 + 1;
  }

  adcInitialized = true;

  // Start conversions
  if (HAL_ADC_Start_DMA(config->handle, adcDMABuf, numChannels) != HAL_OK) {
    return ADC_STATUS_ERROR_DMA;
  }

  // TODO: this only works for ADC1 (with multiple channels), expand to ADCx

  REGISTER_STATIC(ADC, ADC_STATUS_ERROR_DEPENDS);
  Log_Print(logging, "ADC_Init complete\n");
  return ADC_STATUS_OK;
}

//------------------------------------------------------------------------------
ADC_Status_T ADC_Get(const ADC_Channel_T channel, uint16_t* val)
{
  if (channel > numChannels) {
    *val = 0;
    return ADC_STATUS_ERROR_INVALID_CHANNEL;
  }

  ADC_Status_T ret = ADC_STATUS_OK;

  HAL_NVIC_DisableIRQ(adcIrq);

  switch (activeCopyBuffer) {
    case COPY_BUFFER_A_ACTIVE:
      *val = (uint16_t)(copyBufferB[channel] & 0xFFFF);
      break;

    case COPY_BUFFER_B_ACTIVE:
      *val = (uint16_t)(copyBufferA[channel] & 0xFFFF);
      break;

    default:
      *val = 0;
      ret = ADC_STATUS_ERROR_INTERNAL;
  }
  
  HAL_NVIC_EnableIRQ(adcIrq);
  
  return ret;
}

//------------------------------------------------------------------------------
float ADC_ApplyScaling(
    const ADC_Scaling_T* scaling,
    const uint16_t raw)
{
  // deliberate conversion up to int32_t to allow negative values
  int32_t num = raw - scaling->lowerScaling;
  int32_t den = scaling->upperScaling - scaling->lowerScaling;

  float output = (float)num / (float)den;

  if (scaling->saturate) {
    if (output > 1.0f) {
      output = 1.0f;
    } else if (output < 0.0f) {
      output = 0.0f;
    }
  }

  return output;
}

// ------------------- Interrupts -------------------
/**
 * @brief Called when first half of buffer is filled
 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc)
{
  if (hadc != adcHandle) {
    return;
  }

  // copy first half of buffer to active copy buffer
  switch (activeCopyBuffer) {
    case COPY_BUFFER_A_ACTIVE:
      bufferCopy(copyBufferA, adcDMABuf, halfCpltCopyLen);
      break;

    case COPY_BUFFER_B_ACTIVE:
      bufferCopy(copyBufferB, adcDMABuf, halfCpltCopyLen);
      break;

    default:
      // This shouldn't ever happen
      break;
  }
}

/**
 * @brief Called when buffer is completely filled
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
  if (hadc != adcHandle) {
    return;
  }

  // copy second half of buffer to second half of active copy buffer and
  // switch the active buffer
  switch (activeCopyBuffer) {
    case COPY_BUFFER_A_ACTIVE:
      bufferCopy(copyBufferA + cpltCopyOffset,
                 adcDMABuf + cpltCopyOffset,
                 cpltCopyLen);
      activeCopyBuffer = COPY_BUFFER_B_ACTIVE;
      break;

    case COPY_BUFFER_B_ACTIVE:
      bufferCopy(copyBufferB + cpltCopyOffset,
                 adcDMABuf + cpltCopyOffset,
                 cpltCopyLen);
      activeCopyBuffer = COPY_BUFFER_A_ACTIVE;
      break;

    default:
      // This shouldn't ever happen
      activeCopyBuffer = COPY_BUFFER_A_ACTIVE;
  }
}
