/*
 * pdm.c
 *
 *  Created on: 19 Nov 2022
 *      Author: Liam Flaherty
 */

#include "pdm.h"

// ------------------- Private data -------------------
static Logging_T* mLog;
static const TickType_t mBlockTime = 10 / portTICK_PERIOD_MS; // 10ms

// ------------------- Public methods -------------------
PDM_Status_T PDM_Init(Logging_T* logger, PDM_T* pdm)
{
  mLog = logger;
  Log_Print(mLog, "PDM_Init begin\n");
  DEPEND_ON(logger, PDM_STATUS_ERROR_DEPENDS);
  DEPEND_ON(pdm->vehicleState, PDM_STATUS_ERROR_DEPENDS);

  if ((pdm->channels == NULL && pdm->numChannels > 0) ||
      (pdm->channels != NULL && pdm->numChannels == 0)) {
    return PDM_STATUS_ERROR_CONFIG;
  }

  if (pdm->numChannels > VEHICLESTATE_MAXPDM_CHANNELS) {
    return PDM_STATUS_ERROR_CONFIG;
  }

  pdm->initComplete = true;

  REGISTER(pdm, PDM_STATUS_ERROR_DEPENDS);
  Log_Print(mLog, "PDM_Init complete\n");
  return PDM_STATUS_OK;
}

PDM_Status_T PDM_SetOutputEnabled(
    PDM_T* pdm,
    uint8_t channel,
    bool state)
{
  if (channel >= pdm->numChannels) {
    return PDM_STATUS_ERROR_INVALID_CH;
  }

  // Update state storage
  if (!VehicleState_AccessAcquire(pdm->vehicleState)) {
    return PDM_STATUS_ERROR_STATE;
  }
  pdm->vehicleState->data.glv.pdmChState[channel] = state;
  VehicleState_AccessRelease(pdm->vehicleState);

  GPIO_T* pin = pdm->channels[channel].pin;
  GPIO_WritePin(pin, state);

  return PDM_STATUS_OK;
}
