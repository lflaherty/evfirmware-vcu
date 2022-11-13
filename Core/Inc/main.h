/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define P_SW4_Pin GPIO_PIN_2
#define P_SW4_GPIO_Port GPIOE
#define P_SW5_Pin GPIO_PIN_3
#define P_SW5_GPIO_Port GPIOE
#define P_SW6_Pin GPIO_PIN_4
#define P_SW6_GPIO_Port GPIOE
#define GPS_FIX_Pin GPIO_PIN_5
#define GPS_FIX_GPIO_Port GPIOE
#define GPS_PPS_Pin GPIO_PIN_6
#define GPS_PPS_GPIO_Port GPIOE
#define MPIO_EN2_Pin GPIO_PIN_2
#define MPIO_EN2_GPIO_Port GPIOB
#define MPIO_OUT7_Pin GPIO_PIN_11
#define MPIO_OUT7_GPIO_Port GPIOF
#define DOUT_START_B_Pin GPIO_PIN_12
#define DOUT_START_B_GPIO_Port GPIOF
#define MPIO_EN7_Pin GPIO_PIN_13
#define MPIO_EN7_GPIO_Port GPIOF
#define DIN_INV_ERR_Pin GPIO_PIN_14
#define DIN_INV_ERR_GPIO_Port GPIOF
#define MPIO_OUT6_Pin GPIO_PIN_15
#define MPIO_OUT6_GPIO_Port GPIOF
#define DIN_WS_R_Pin GPIO_PIN_0
#define DIN_WS_R_GPIO_Port GPIOG
#define MPIO_EN6_Pin GPIO_PIN_1
#define MPIO_EN6_GPIO_Port GPIOG
#define DIN_WS_F_Pin GPIO_PIN_7
#define DIN_WS_F_GPIO_Port GPIOE
#define MPIO_OUT3_Pin GPIO_PIN_8
#define MPIO_OUT3_GPIO_Port GPIOE
#define DIN_START_B_Pin GPIO_PIN_9
#define DIN_START_B_GPIO_Port GPIOE
#define MPIO_EN3_Pin GPIO_PIN_10
#define MPIO_EN3_GPIO_Port GPIOE
#define MPIO_OUT2_Pin GPIO_PIN_11
#define MPIO_OUT2_GPIO_Port GPIOE
#define MPIO_EN1_Pin GPIO_PIN_12
#define MPIO_EN1_GPIO_Port GPIOB
#define MPIO_OUT1_Pin GPIO_PIN_13
#define MPIO_OUT1_GPIO_Port GPIOB
#define MPIO_EN4_Pin GPIO_PIN_10
#define MPIO_EN4_GPIO_Port GPIOD
#define MPIO_OUT4_Pin GPIO_PIN_11
#define MPIO_OUT4_GPIO_Port GPIOD
#define IMU_INT_Pin GPIO_PIN_14
#define IMU_INT_GPIO_Port GPIOD
#define IMU_INT_EXTI_IRQn EXTI15_10_IRQn
#define IMU_ADDR0_Pin GPIO_PIN_15
#define IMU_ADDR0_GPIO_Port GPIOD
#define MPIO_EN5_Pin GPIO_PIN_2
#define MPIO_EN5_GPIO_Port GPIOG
#define MPIO_OUT5_Pin GPIO_PIN_3
#define MPIO_OUT5_GPIO_Port GPIOG
#define EEPROM_ADDR2_Pin GPIO_PIN_4
#define EEPROM_ADDR2_GPIO_Port GPIOG
#define EEPROM_ADDR1_Pin GPIO_PIN_5
#define EEPROM_ADDR1_GPIO_Port GPIOG
#define EEPROM_ADDR0_Pin GPIO_PIN_6
#define EEPROM_ADDR0_GPIO_Port GPIOG
#define EEPROM_WP_Pin GPIO_PIN_7
#define EEPROM_WP_GPIO_Port GPIOG
#define MPIO_EN8_Pin GPIO_PIN_8
#define MPIO_EN8_GPIO_Port GPIOG
#define MPIO_OUT8_Pin GPIO_PIN_6
#define MPIO_OUT8_GPIO_Port GPIOC
#define SDMMC_DET_Pin GPIO_PIN_11
#define SDMMC_DET_GPIO_Port GPIOA
#define STATUS_LED_Pin GPIO_PIN_12
#define STATUS_LED_GPIO_Port GPIOA
#define ECU_SW_Error_Pin GPIO_PIN_4
#define ECU_SW_Error_GPIO_Port GPIOD
#define GPIO_EXTI9_SDCBMS_Pin GPIO_PIN_9
#define GPIO_EXTI9_SDCBMS_GPIO_Port GPIOG
#define GPIO_EXTI9_SDCBMS_EXTI_IRQn EXTI9_5_IRQn
#define GPIO_EXTI10_SDCIMD_Pin GPIO_PIN_10
#define GPIO_EXTI10_SDCIMD_GPIO_Port GPIOG
#define GPIO_EXTI10_SDCIMD_EXTI_IRQn EXTI15_10_IRQn
#define GPIO_EXTI15_SDCBSPD_Pin GPIO_PIN_15
#define GPIO_EXTI15_SDCBSPD_GPIO_Port GPIOG
#define GPIO_EXTI15_SDCBSPD_EXTI_IRQn EXTI15_10_IRQn
#define GPIO_EXTI4_SDCOUT_Pin GPIO_PIN_4
#define GPIO_EXTI4_SDCOUT_GPIO_Port GPIOB
#define GPIO_EXTI4_SDCOUT_EXTI_IRQn EXTI4_IRQn
#define P_SW1_Pin GPIO_PIN_9
#define P_SW1_GPIO_Port GPIOB
#define P_SW2_Pin GPIO_PIN_0
#define P_SW2_GPIO_Port GPIOE
#define P_SW3_Pin GPIO_PIN_1
#define P_SW3_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
