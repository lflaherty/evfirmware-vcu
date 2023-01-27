/*
 * initialize.h
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#ifndef INC_INITIALIZE_H_
#define INC_INITIALIZE_H_

#include "stm32f7xx_hal.h"

/**
 * @brief Initializes "System" and "Application" components for this ECU and
 * starts scheduling.
 * This method invokes the RTOS scheduler, and should never exit.
 */
void ECU_Init(void);

/**
 * @brief IRQ Handler for timer elapsed callback.
 * 
 * @param htim TIM event
 */
void TIM_IRQHandler(TIM_HandleTypeDef *htim);


#endif /* INC_INITIALIZE_H_ */
