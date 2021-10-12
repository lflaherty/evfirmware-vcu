/*
 * MockStm32f7xx_hal_can.c
 *
 *  Created on: Oct 10 2021
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_can.h"

#include <string.h>

// ------------------- Static data -------------------
static HAL_StatusTypeDef mStatusGetRxMessage = HAL_OK;
static HAL_StatusTypeDef mStatusConfigFilter = HAL_OK;
static HAL_StatusTypeDef mStatusStart = HAL_OK;
static HAL_StatusTypeDef mStatusActivateNotification = HAL_OK;
static HAL_StatusTypeDef mStatusAddTxMessage = HAL_OK;

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

    return mStatusGetRxMessage;
}

HAL_StatusTypeDef HAL_CAN_ConfigFilter(CAN_HandleTypeDef *hcan, CAN_FilterTypeDef *sFilterConfig)
{
    (void)hcan;
    (void)sFilterConfig;
    return mStatusConfigFilter;
}

HAL_StatusTypeDef HAL_CAN_Start(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    return mStatusStart;
}

HAL_StatusTypeDef HAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t ActiveITs)
{
    (void)hcan;
    (void)ActiveITs;
    return mStatusActivateNotification;
}

HAL_StatusTypeDef HAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox)
{
    (void)hcan;
    (void)pTxMailbox;

    mMsgData = aData;
    mDataSize = pHeader->DLC;

    return mStatusAddTxMessage;
}

void mockSetHALCANRxMessage(void* data, size_t dataSize, CAN_RxHeaderTypeDef* header)
{
    mMsgData = data;
    mDataSize = dataSize;
    mHeaderData = *header;
}

void mockSet_HAL_CAN_AllStatus(HAL_StatusTypeDef status)
{
    mStatusGetRxMessage = status;
    mStatusConfigFilter = status;
    mStatusStart = status;
    mStatusActivateNotification = status;
    mStatusAddTxMessage = status;
}

void mockSet_HAL_CAN_GetRxMessage_Status(HAL_StatusTypeDef status)
{
    mStatusGetRxMessage = status;
}

void mockSet_HAL_CAN_ConfigFilter_Status(HAL_StatusTypeDef status)
{
    mStatusConfigFilter = status;
}

void mockSet_HAL_CAN_Start_Status(HAL_StatusTypeDef status)
{
    mStatusStart = status;
}

void mockSet_HAL_CAN_ActivateNotification_Status(HAL_StatusTypeDef status)
{
    mStatusActivateNotification = status;
}

void mockSet_HAL_CAN_AddTxMessage_Status(HAL_StatusTypeDef status)
{
    mStatusAddTxMessage = status;
}