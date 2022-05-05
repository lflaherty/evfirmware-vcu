/*
 * MockTasktimer.h
 *
 *  Created on: Oct 17 2021
 *      Author: Liam Flaherty
 */

#ifndef _MOCK_TIME_TASKTIMER_TASKTIMER_H_
#define _MOCK_TIME_TASKTIMER_TASKTIMER_H_

// Redefine methods to be mocked
#define TaskTimer_Init stub_TaskTimer_Init
#define TaskTimer_RegisterTask stub_TaskTimer_RegisterTask
#define TaskTimer_TIM_PeriodElapsedCallback stub_TaskTimer_TIM_PeriodElapsedCallback

// Bring in the header to be mocked
#include "time/tasktimer/tasktimer.h"

// ============= Mock control methods =============
void mockSet_TaskTimer_Init_Status(TaskTimer_Status_T status);
void mockSet_TaskTimer_RegisterTask_Status(TaskTimer_Status_T status);

#endif // _MOCK_TIME_TASKTIMER_TASKTIMER_H_