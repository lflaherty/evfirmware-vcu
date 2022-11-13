/*
 * MockStm32f7xx_hal_gpio.h
 * Some excerpts from stm32f7xx_hal_gpio.h in STM32 HAL.
 *
 *  Created on: Oct 15 2021
 *      Author: Liam Flaherty
 */

#ifndef MOCK_STM32F7xx_GPIO_H_
#define MOCK_STM32F7xx_GPIO_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "MockStm32f7xx_hal_def.h"

// ================== Define types ==================
typedef struct
{
    // Empty for mock
    uint8_t tmp;
} GPIO_TypeDef;

typedef struct
{
    GPIO_TypeDef* Instance;
} GPIO_HandleTypeDef;

/** 
  * @brief  GPIO Bit SET and Bit RESET enumeration 
  */
typedef enum
{
  GPIO_PIN_RESET = 0,
  GPIO_PIN_SET
} GPIO_PinState;


// ================== Define methods ==================
GPIO_PinState stubHAL_GPIO_ReadPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void stubHAL_GPIO_WritePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, GPIO_PinState PinState);
void stubHAL_GPIO_TogglePin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

// Replace real methods with mock stubs
#define HAL_GPIO_ReadPin stubHAL_GPIO_ReadPin
#define HAL_GPIO_WritePin stubHAL_GPIO_WritePin
#define HAL_GPIO_TogglePin stubHAL_GPIO_TogglePin

// ================== Mock control methods ==================
void mock_GPIO_RegisterPin(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
void mockSet_GPIO_Asserted(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, bool asserted);
bool mockGet_GPIO_Asserted(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

#endif
