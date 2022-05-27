/*
 * vehicleStateManager.c
 *
 *  Created on: May 11 2022
 *      Author: Liam Flaherty
 */

#include "vehicleStateManager.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 100 / portTICK_PERIOD_MS; // 100ms

// ------------------- Private methods -------------------

// ------------------- Public methods -------------------
VehicleStateManager_Status_T VehicleStateManager_Init(Logging_T* logger, VehicleStateManager_T* sm)
{
  mLog = logger;
  logPrintS(mLog, "VehicleStateManager_Init begin\n", LOGGING_DEFAULT_BUFF_LEN);

  // TODO
  (void)sm;

  logPrintS(mLog, "VehicleStateManager_Init complete\n", LOGGING_DEFAULT_BUFF_LEN);
  return STATEMANAGER_STATUS_OK;
}