/*
 * MockStm32f7xx_hal_can.c
 *
 *  Created on: Oct 10 2021
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_can.h"

#include <string.h>

// ------------------- Static data -------------------
static HAL_StatusTypeDef mStatus = HAL_OK;
static void* mMsgData; // data to use for CAN message
static size_t mDataSize;
static CAN_RxHeaderTypeDef mHeaderData;

// ------------------- Methods -------------------
HAL_StatusTypeDef HAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t RxFifo, CAN_RxHeaderTypeDef *pHeader, uint8_t aData[])
{
    (void)hcan;
    (void)RxFifo;
    
    memcpy(aData, mMsgData, mDataSize);
    *pHeader = mHeaderData;

    return mStatus;
}

HAL_StatusTypeDef HAL_CAN_ConfigFilter(CAN_HandleTypeDef *hcan, CAN_FilterTypeDef *sFilterConfig)
{
    (void)hcan;
    (void)sFilterConfig;
    return mStatus;
}

HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    return mStatus;
}

HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t ActiveITs)
{
    (void)hcan;
    (void)ActiveITs;
    return mStatus;
}

HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox)
{
    (void)hcan;
    (void)pTxMailbox;

    mMsgData = aData;
    mDataSize = pHeader->DLC;

    return mStatus;
}

void mockSetHALCANRxMessage(void* data, size_t dataSize, CAN_RxHeaderTypeDef* header)
{
    mMsgData = data;
    mDataSize = dataSize;
    mHeaderData = *header;
}

void mockSetHALCANStatus(HAL_StatusTypeDef status)
{
    mStatus = status;
}