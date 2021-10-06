/*
 * initialize.h
 *
 *  Created on: 6 Oct 2021
 *      Author: Liam Flaherty
 */

#ifndef INC_INITIALIZE_H_
#define INC_INITIALIZE_H_


typedef enum
{
  ECU_INIT_OK     = 0x00U,
  ECU_INIT_ERROR  = 0x01
} ECU_Init_Status_T;

/**
 * @brief Initializes "System" and "Application" components for this ECU
 */
ECU_Init_Status_T ECU_Init(void);


#endif /* INC_INITIALIZE_H_ */
