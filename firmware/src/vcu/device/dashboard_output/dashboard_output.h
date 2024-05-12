/*
 * dashboard_output.h
 *
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_DASHBOARD_OUTPUT_DASHBOARD_OUTPUT_H_
#define DEVICE_DASHBOARD_OUTPUT_DASHBOARD_OUTPUT_H_

#include <stdbool.h>

#include "lib/depends/depends.h"
#include "lib/logging/logging.h"
#include "io/gpio/gpio.h"
#include "vehicleInterface/vehicleState/vehicleState.h"

typedef enum
{
  DASHBOARDOUT_OK       = 0x00,
  DASHBOARDOUT_ERROR    = 0x01,
  DASHBOARDOUT_DEPENDS  = 0x02,
} DashboardOut_Status_T;

typedef struct
{
  GPIO_T* output_pin;
  VehicleState_T* vehicleState;
  Logging_T* log;

  // Internal
  REGISTERED_MODULE();
} DashboardOut_T;

/**
 * @brief Initialize dashboard output control module.
 * 
 * @param dash Dash control struct 
 * @return DASHBOARDOUT_OK if no error.
 */
DashboardOut_Status_T DashboardOut_Init(
  DashboardOut_T* dash);

/**
 * @brief Set the current dashboard LED.
 * 
 * @param dash Dash control struct
 * @param on LED state
 * @return DASHBOARDOUT_OK if no error. 
 */
DashboardOut_Status_T DashboardOut_Set(DashboardOut_T* dash, const bool on);

#endif  // DEVICE_DASHBOARD_OUTPUT_DASHBOARD_OUTPUT_H_
