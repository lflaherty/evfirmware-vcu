/*
 * MockStm32f7xx_hal_crc.h
 * Some excerpts from stm32f7xx_hal_crc.h in STM32 HAL.
 *
 *  Created on: Jul 28 2022
 *      Author: Liam Flaherty
 */

#ifndef MOCK_STM32F7xx_CRC_H_
#define MOCK_STM32F7xx_CRC_H_

#include <stdint.h>
#include <stddef.h>
#include "MockStm32f7xx_hal_def.h"

// ================== Define types ==================
typedef struct
{
    // Empty for mock
} CRC_TypeDef;

typedef struct
{
    CRC_TypeDef* Instance;
    uint32_t InputDataFormat;
} CRC_HandleTypeDef;

// Duplicated defines from HAL:

/** @defgroup CRC_Input_Buffer_Format Input Buffer Format
  * @{
  */
/* WARNING: CRC_INPUT_FORMAT_UNDEFINED is created for reference purposes but
 * an error is triggered in HAL_CRC_Init() if InputDataFormat field is set
 * to CRC_INPUT_FORMAT_UNDEFINED: the format MUST be defined by the user for
 * the CRC APIs to provide a correct result */
#define CRC_INPUTDATA_FORMAT_UNDEFINED             0x00000000U  /*!< Undefined input data format    */
#define CRC_INPUTDATA_FORMAT_BYTES                 0x00000001U  /*!< Input data in byte format      */
#define CRC_INPUTDATA_FORMAT_HALFWORDS             0x00000002U  /*!< Input data in half-word format */
#define CRC_INPUTDATA_FORMAT_WORDS                 0x00000003U  /*!< Input data in word format      */


// ================== Define methods ==================
uint32_t stubHAL_CRC_Calculate(CRC_HandleTypeDef *hcrc, uint32_t pBuffer[], uint32_t BufferLength);

// Replace real methods with mock stubs
#define HAL_CRC_Calculate stubHAL_CRC_Calculate

// ================== Mock control methods ==================
void mockSet_CRC(uint32_t crc);

#endif