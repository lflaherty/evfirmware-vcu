/**
 * MockTaskTimer.h
 * 
 *  Created on: Oct 17 2021
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_MOCKTASKTIMER_H_
#define _MOCK_MOCKTASKTIMER_H_

// Bring in the header to be mocked
#include "time/tasktimer/tasktimer.h"

void mockSet_TaskTimer_Init_Status(TaskTimer_Status_T status);
void mockSet_TaskTimer_RegisterTask_Status(TaskTimer_Status_T status);

#endif