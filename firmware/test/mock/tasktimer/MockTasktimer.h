/*
 * MockTasktimer.h
 *
 *  Created on: Oct 17 2021
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_TIME_TASKTIMER_TASKTIMER_H_
#define _MOCK_TIME_TASKTIMER_TASKTIMER_H_

// Bring in the header to be mocked
#include "tasktimer/tasktimer.h"

// ============= Mock control methods =============
void mockSet_TaskTimer_Init_Status(TaskTimer_Status_T status);
void mockSet_TaskTimer_RegisterTask_Status(TaskTimer_Status_T status);

#endif // _MOCK_TIME_TASKTIMER_TASKTIMER_H_
