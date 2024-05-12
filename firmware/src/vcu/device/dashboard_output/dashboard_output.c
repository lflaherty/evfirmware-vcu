/*
 * dashboard_output.c
 *
 *      Author: Liam Flaherty
 */

#include "dashboard_output.h"

// ------------------- Public methods -------------------
DashboardOut_Status_T DashboardOut_Init(DashboardOut_T* dash)
{
  DEPEND_ON(dash->log, DASHBOARDOUT_DEPENDS);
  DEPEND_ON(dash->vehicleState, DASHBOARDOUT_DEPENDS);
  Log_Print(dash->log, "DashboardOut_Init begin\n");

  GPIO_WritePin(dash->output_pin, false);

  Log_Print(dash->log, "DashboardOut_Init complete\n");
  return DASHBOARDOUT_OK;
}

DashboardOut_Status_T DashboardOut_Set(DashboardOut_T* dash, const bool on)
{
  GPIO_WritePin(dash->output_pin, on);
  return DASHBOARDOUT_OK;
}
