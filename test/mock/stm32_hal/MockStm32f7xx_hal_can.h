/*
 * MockStm32f7xx_hal_can.h
 * Some excerpts from stm32f7xx_hal_can.h in STM32 HAL.
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#ifndef MOCK_STM32F7xx_HAL_can_H_
#define MOCK_STM32F7xx_HAL_can_H_

#include <stdint.h>
#include <stddef.h>
#include "MockStm32f7xx_hal_def.h"

// ================== Define types ==================
typedef struct
{
    // Empty for mock
} CAN_TypeDef;

typedef struct 
{
  CAN_TypeDef* Instance;
} CAN_HandleTypeDef;

/**
  * @brief  CAN filter configuration structure definition
  * Duplicated from HAL
  */
typedef struct
{
  uint32_t FilterIdHigh;
  uint32_t FilterIdLow;
  uint32_t FilterMaskIdHigh;
  uint32_t FilterMaskIdLow;
  uint32_t FilterFIFOAssignment;
  uint32_t FilterBank;
  uint32_t FilterMode;
  uint32_t FilterScale;
  uint32_t FilterActivation;
  uint32_t SlaveStartFilterBank;
} CAN_FilterTypeDef;

/**
  * @brief  CAN Tx message header structure definition
  * Duplicated from HAL
  */
typedef struct
{
  uint32_t StdId;
  uint32_t ExtId;
  uint32_t IDE;
  uint32_t RTR;
  uint32_t DLC;
  FunctionalState TransmitGlobalTime;
} CAN_TxHeaderTypeDef;

/**
  * @brief  CAN Rx message header structure definition
  * Duplicated from HAL
  */
typedef struct
{
    uint32_t StdId;
    uint32_t ExtId;
    uint32_t IDE;
    uint32_t RTR;
    uint32_t DLC;
    uint32_t Timestamp;
    uint32_t FilterMatchIndex;
} CAN_RxHeaderTypeDef;

// Duplicated defines from HAL:

/** @defgroup CAN_filter_mode CAN Filter Mode
  * @{
  */
#define CAN_FILTERMODE_IDMASK       (0x00000000U)  /*!< Identifier mask mode */
#define CAN_FILTERMODE_IDLIST       (0x00000001U)  /*!< Identifier list mode */
/**
  * @}
  */

/** @defgroup CAN_filter_scale CAN Filter Scale
  * @{
  */
#define CAN_FILTERSCALE_16BIT       (0x00000000U)  /*!< Two 16-bit filters */
#define CAN_FILTERSCALE_32BIT       (0x00000001U)  /*!< One 32-bit filter  */
/**
  * @}
  */

/** @defgroup CAN_filter_activation CAN Filter Activation
  * @{
  */
#define CAN_FILTER_DISABLE          (0x00000000U)  /*!< Disable filter */
#define CAN_FILTER_ENABLE           (0x00000001U)  /*!< Enable filter  */
/**
  * @}
  */

/** @defgroup CAN_filter_FIFO CAN Filter FIFO
  * @{
  */
#define CAN_FILTER_FIFO0            (0x00000000U)  /*!< Filter FIFO 0 assignment for filter x */
#define CAN_FILTER_FIFO1            (0x00000001U)  /*!< Filter FIFO 1 assignment for filter x */
/**
  * @}
  */

/** @defgroup CAN_identifier_type CAN Identifier Type
  * @{
  */
#define CAN_ID_STD                  (0x00000000U)  /*!< Standard Id */
#define CAN_ID_EXT                  (0x00000004U)  /*!< Extended Id */
/**
  * @}
  */

/** @defgroup CAN_remote_transmission_request CAN Remote Transmission Request
  * @{
  */
#define CAN_RTR_DATA                (0x00000000U)  /*!< Data frame   */
#define CAN_RTR_REMOTE              (0x00000002U)  /*!< Remote frame */
/**
  * @}
  */

/** @defgroup CAN_receive_FIFO_number CAN Receive FIFO Number
  * @{
  */
#define CAN_RX_FIFO0                (0x00000000U)  /*!< CAN receive FIFO 0 */
#define CAN_RX_FIFO1                (0x00000001U)  /*!< CAN receive FIFO 1 */
/**
  * @}
  */


/* Transmit Interrupt */
#define CAN_IT_TX_MAILBOX_EMPTY     ((uint32_t)0)

/* Receive Interrupts */
#define CAN_IT_RX_FIFO0_MSG_PENDING ((uint32_t)1)
#define CAN_IT_RX_FIFO0_FULL        ((uint32_t)2)
#define CAN_IT_RX_FIFO0_OVERRUN     ((uint32_t)3)
#define CAN_IT_RX_FIFO1_MSG_PENDING ((uint32_t)4)
#define CAN_IT_RX_FIFO1_FULL        ((uint32_t)5)
#define CAN_IT_RX_FIFO1_OVERRUN     ((uint32_t)6)



// ================== Define methods ==================
HAL_StatusTypeDef stubHAL_CAN_GetRxMessage(CAN_HandleTypeDef *hcan, uint32_t RxFifo, CAN_RxHeaderTypeDef *pHeader, uint8_t aData[]);
HAL_StatusTypeDef stubHAL_CAN_ConfigFilter(CAN_HandleTypeDef *hcan, CAN_FilterTypeDef *sFilterConfig);
HAL_StatusTypeDef stubHAL_CAN_Start(CAN_HandleTypeDef *hcan);
HAL_StatusTypeDef stubHAL_CAN_ActivateNotification(CAN_HandleTypeDef *hcan, uint32_t ActiveITs);
HAL_StatusTypeDef stubHAL_CAN_AddTxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef *pHeader, uint8_t aData[], uint32_t *pTxMailbox);

// Replace real methods with mock stubs
#define HAL_CAN_GetRxMessage stubHAL_CAN_GetRxMessage
#define HAL_CAN_ConfigFilter stubHAL_CAN_ConfigFilter
#define HAL_CAN_Start stubHAL_CAN_Start
#define HAL_CAN_ActivateNotification stubHAL_CAN_ActivateNotification
#define HAL_CAN_AddTxMessage stubHAL_CAN_AddTxMessage

// ================== Mock control methods ==================
/**
 * @brief Sets the queue to return this data upon next xQueueReceive
 * @param data Pointer to data to use. This will be used to control size of data copying
 */
void mockSetHALCANMessage(void* data, CAN_RxHeaderTypeDef* header);

void mockSet_HAL_CAN_AllStatus(HAL_StatusTypeDef status);
void mockSet_HAL_CAN_GetRxMessage_Status(HAL_StatusTypeDef status);
void mockSet_HAL_CAN_ConfigFilter_Status(HAL_StatusTypeDef status);
void mockSet_HAL_CAN_Start_Status(HAL_StatusTypeDef status);
void mockSet_HAL_CAN_ActivateNotification_Status(HAL_StatusTypeDef status);
void mockSet_HAL_CAN_AddTxMessage_Status(HAL_StatusTypeDef status);

#endif