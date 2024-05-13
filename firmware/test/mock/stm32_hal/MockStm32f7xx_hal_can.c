/*
 * MockStm32f7xx_hal_can.c
 *
 *  Created on: Oct 10 2021
 *      Author: Liam Flaherty
 */
#include "MockStm32f7xx_hal_can.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

// ------------------- Static data -------------------
static HAL_StatusTypeDef mStatusGetRxMessage = HAL_OK;
static HAL_StatusTypeDef mStatusConfigFilter = HAL_OK;
static HAL_StatusTypeDef mStatusStart = HAL_OK;
static HAL_StatusTypeDef mStatusActivateNotification = HAL_OK;
static HAL_StatusTypeDef mStatusAddTxMessage = HAL_OK;

// Tx
#define NUM_MAILBOXES 3U
static bool txMailboxInUse[NUM_MAILBOXES] = { 0 };
static uint8_t mTxMsgData[NUM_MAILBOXES][8]; // data to use for CAN message
static CAN_TxHeaderTypeDef mTxHeaderData[NUM_MAILBOXES];

// Rx
#define RECV_FIFO_SIZE 32U
static uint32_t mRxNext = 0U;
static uint32_t mRxPending = 0U;
static uint8_t mRxMsgData[NUM_MAILBOXES][8]; // data to use for CAN message
static CAN_RxHeaderTypeDef mRxHeaderData[RECV_FIFO_SIZE];

// ------------------- Helpers -------------------
size_t getFirstFreeTxMailboxIndex(void)
{
    for (size_t i = 0; i < NUM_MAILBOXES; ++i) {
        if (false == txMailboxInUse[i]) {
            return i;
        }
    }

    assert(false); // this is only reached if there's a programming error
    return 0;
}

uint32_t numFreeMailboxes(void)
{
    uint32_t n = 0;
    for (size_t i = 0; i < NUM_MAILBOXES; ++i) {
        if (txMailboxInUse[i] == false) {
            n++;
        }
    }
    
    return n;
}

// ------------------- Methods -------------------
HAL_StatusTypeDef stubHAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t RxFifo, CAN_RxHeaderTypeDef *pHeader, uint8_t aData[])
{
    (void)hcan;
    (void)RxFifo;

    if (mRxNext == mRxPending) {
        // No more data in FIFO
        return HAL_ERROR;
    }

    *pHeader = mRxHeaderData[mRxNext];
    memcpy(aData, mRxMsgData[mRxNext], mRxHeaderData[mRxNext].DLC);

    mRxNext++;
    return mStatusGetRxMessage;
}

HAL_StatusTypeDef stubHAL_CAN_ConfigFilter(CAN_HandleTypeDef *hcan, CAN_FilterTypeDef *sFilterConfig)
{
    (void)hcan;
    (void)sFilterConfig;
    return mStatusConfigFilter;
}

HAL_StatusTypeDef stubHAL_CAN_Start(CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    return mStatusStart;
}

HAL_StatusTypeDef stubHAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t ActiveITs)
{
    (void)hcan;
    (void)ActiveITs;
    return mStatusActivateNotification;
}

HAL_StatusTypeDef stubHAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox)
{
    (void)hcan;
    (void)pTxMailbox;

    if (0U == numFreeMailboxes()) {
        return HAL_ERROR;
    }

    size_t mailbox = getFirstFreeTxMailboxIndex();
    txMailboxInUse[mailbox] = true;

    memcpy(mTxMsgData[mailbox], aData, pHeader->DLC);
    mTxHeaderData[mailbox] = *pHeader;

    return mStatusAddTxMessage;
}

uint32_t stubHAL_CAN_GetTxMailboxesFreeLevel(const CAN_HandleTypeDef *hcan)
{
    (void)hcan;
    return numFreeMailboxes();
}

uint32_t stubHAL_CAN_GetRxFifoFillLevel(const CAN_HandleTypeDef *hcan, uint32_t RxFifo)
{
    (void)hcan;
    (void)RxFifo;

    return (mRxPending - mRxNext);
}

void mockAddHALCANRxMessage(
        uint32_t msgId,
        uint8_t* data,
        uint32_t dlc)
{
    assert(mRxPending < RECV_FIFO_SIZE);

    memcpy(mRxMsgData[mRxPending], data, dlc);
    mRxHeaderData[mRxPending].StdId = msgId;
    mRxHeaderData[mRxPending].ExtId = msgId;
    mRxHeaderData[mRxPending].RTR = CAN_RTR_DATA;
    mRxHeaderData[mRxPending].IDE = CAN_ID_STD;
    mRxHeaderData[mRxPending].DLC = dlc;
    mRxHeaderData[mRxPending].Timestamp = 0U;
    mRxHeaderData[mRxPending].FilterMatchIndex = 0U;
    mRxPending++;
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

uint32_t mockGet_HAL_CAN_NumTxMailboxesInUse(void)
{
    return NUM_MAILBOXES - numFreeMailboxes();
}

CAN_TxHeaderTypeDef* mockGet_HAL_CAN_TxHeader(uint32_t mailbox)
{
    assert(mailbox < NUM_MAILBOXES);
    return &mTxHeaderData[mailbox];
}

uint8_t* mockGet_HAL_CAN_TxData(uint32_t mailbox)
{
    return mTxMsgData[mailbox];
}

void mockClear_HAL_CAN_TxMailboxes(void)
{
    for (size_t i = 0; i < NUM_MAILBOXES; ++i) {
        txMailboxInUse[i] = false;
    }
}

void mockClear_HAL_CAN_RxFifo(void)
{
    mRxNext = 0U;
    mRxPending = 0U;
}
