/*
 * MockStm32f7xx_hal_can.h
 *
 *  Created on: Oct 10 2021
 *      Author: Liam Flaherty
 */

#ifndef __STM32F7xx_HAL_DEF
#define __STM32F7xx_HAL_DEF

/** 
  * @brief  HAL Status structures definition  
  * Duplicated from HAL
  */  
typedef enum 
{
  HAL_OK       = 0x00U,
  HAL_ERROR    = 0x01U,
  HAL_BUSY     = 0x02U,
  HAL_TIMEOUT  = 0x03U
} HAL_StatusTypeDef;

typedef enum
{
  DISABLE = 0U,
  ENABLE = !DISABLE
} FunctionalState;

#endif