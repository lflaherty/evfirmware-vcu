/*
 * discretesense.h
 *
 * Driver for discrete (ADC/GPI) sensors at uniform sampling rate.
 * The unit conversions will also be performed here.
 * Various sensors of same type lumped together to save on RTOS overhead.
 * 
 * Sensors:
 *  * Accelerator pedals A/B
 *  * Brake pedals (front & rear)
 *  * Dashboard input button
 * 
 *  Created on: 21 May 2023
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_DISCRETESENSE_DISCRETESENSE_H_
#define DEVICE_DISCRETESENSE_DISCRETESENSE_H_

#include "depends/depends.h"
#include "logging/logging.h"
#include "adc/adc.h"
#include "gpio/gpio.h"

#include "vehicleInterface/vehicleState/vehicleState.h"

typedef enum
{
  DISCRETESENSE_STATUS_OK             = 0,
  DISCRETESENSE_STATUS_ERROR_INIT     = 1,
  DISCRETESENSE_STATUS_ERROR_DEPENDS  = 2,
} DiscreteSense_Status_T;

#define DISCRETESENSE_STACK_SIZE 2000U
#define DISCRETESENSE_TASK_PRIORITY 10U

typedef struct
{
  Logging_T* logger;
  VehicleState_T* state; // Vehicle state object to push data to
  
  // sense inputs:
  ADC_Channel_T adcAccelPedalA;
  ADC_Channel_T adcAccelPedalB;
  ADC_Channel_T adcBrakeFront;
  ADC_Channel_T adcBrakeRear;
  GPIO_T* gpioDashboardButton;

  // scaling for ADC inputs:
  ADC_Scaling_T scalingAccelPedalA;
  ADC_Scaling_T scalingAccelPedalB;
  ADC_Scaling_T scalingBrakeFront;
  ADC_Scaling_T scalingBrakeRear;

  // ******* Internal use *******
  // RTOS task
  TaskHandle_t taskHandle;
  StaticTask_t taskBuffer;
  StackType_t taskStack[DISCRETESENSE_STACK_SIZE];

  REGISTERED_MODULE();
} DiscreteSense_T;

/**
 * @brief Initialize discrete sense driver/RTOS task.
 * 
 * @param module Pointer to DiscreteSense struct.
 * @return Return status. DISCRETESENSE_STATUS_OK for success. See
 * DiscreteSense_Status_T for more.
 */
DiscreteSense_Status_T DiscreteSense_Init(DiscreteSense_T* module);

#endif // DEVICE_DISCRETESENSE_DISCRETESENSE_H_
