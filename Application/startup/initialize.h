/*
 * initialize.h
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#ifndef INC_INITIALIZE_H_
#define INC_INITIALIZE_H_

/**
 * @brief Initializes "System" and "Application" components for this ECU and
 * starts scheduling.
 * This method invokes the RTOS scheduler, and should never exit.
 */
void ECU_Init(void);


#endif /* INC_INITIALIZE_H_ */
