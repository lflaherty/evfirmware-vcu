/*
 * MockStm32f7xx_hal_uart.h
 *
 *  Created on: 23 Aug 2021
 *      Author: Liam Flaherty
 */

#ifndef MOCK_STM32F7xx_HAL_UART_H_
#define MOCK_STM32F7xx_HAL_UART_H_

#include <stdint.h>
#include <stddef.h>
#include "MockStm32f7xx_hal_def.h"

// ================== Define types ==================
typedef struct
{
    // Empty for mock
} USART_TypeDef;

typedef struct 
{
    USART_TypeDef* Instance;
} UART_HandleTypeDef;

// ================== Define methods ==================
HAL_StatusTypeDef stubHAL_UART_Init(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stubHAL_UART_DeInit(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stubHAL_UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef stubHAL_UART_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t Timeout);
HAL_StatusTypeDef stubHAL_UART_Transmit_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef stubHAL_UART_Receive_IT(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef stubHAL_UART_Transmit_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef stubHAL_UART_Receive_DMA(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
HAL_StatusTypeDef stubHAL_UART_DMAPause(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stubHAL_UART_DMAResume(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stubHAL_UART_DMAStop(UART_HandleTypeDef *huart);
/* Transfer Abort functions */
HAL_StatusTypeDef stubHAL_UART_Abort(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stubHAL_UART_AbortTransmit(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stubHAL_UART_AbortReceive(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stubHAL_UART_Abort_IT(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stubHAL_UART_AbortTransmit_IT(UART_HandleTypeDef *huart);
HAL_StatusTypeDef stubHAL_UART_AbortReceive_IT(UART_HandleTypeDef *huart);

// Replace real methods with mock stubs
#define HAL_UART_Init stubHAL_UART_Init
#define HAL_UART_DeInit stubHAL_UART_DeInit
#define HAL_UART_Transmit stubHAL_UART_Transmit
#define HAL_UART_Receive stubHAL_UART_Receive
#define HAL_UART_Receive stubHAL_UART_Receive
#define HAL_UART_Receive_IT stubHAL_UART_Receive_IT
#define HAL_UART_Transmit_DMA stubHAL_UART_Transmit_DMA
#define HAL_UART_Receive_DMA stubHAL_UART_Receive_DMA
#define HAL_UART_DMAPause stubHAL_UART_DMAPause
#define HAL_UART_DMAResume stubHAL_UART_DMAResume
#define HAL_UART_DMAStop stubHAL_UART_DMAStop
#define HAL_UART_Abort stubHAL_UART_Abort
#define HAL_UART_AbortTransmit stubHAL_UART_AbortTransmit
#define HAL_UART_AbortReceive stubHAL_UART_AbortReceive
#define HAL_UART_Abort_IT stubHAL_UART_Abort_IT
#define HAL_UART_AbortTransmit_IT stubHAL_UART_AbortTransmit_IT
#define HAL_UART_AbortReceive_IT stubHAL_UART_AbortReceive_IT

// ISRs
void HAL_UART_RxHalfCpltCallback(UART_HandleTypeDef* huart);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart);
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart);

// ================== Mock control methods ==================
void mockSet_HAL_UART_Init_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_DeInit_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_All_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_Transmit_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_Receive_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_Transmit_IT_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_Receive_IT_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_Transmit_DMA_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_Receive_DMA_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_DMAPause_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_DMAResume_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_DMAStop_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_Abort_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_AbortTransmit_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_AbortReceive_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_Abort_IT_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_AbortTransmit_IT_Status(HAL_StatusTypeDef status);
void mockSet_HAL_UART_AbortReceive_IT_Status(HAL_StatusTypeDef status);

#endif