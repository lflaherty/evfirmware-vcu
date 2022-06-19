/*
 * MockStateMachine.c
 *
 *  Created on: 19 Jun 2022
 *      Author: Liam Flaherty
 */

#include "MockStateMachine.h"
#include <stdint.h>

// ------------------- Static data -------------------
static uint32_t mNumSteps = 0;

// ------------------- Methods -------------------
void stub_VSM_Init(Logging_T* logger, VSM_T* vsm)
{
    (void)logger;
    (void)vsm;
}

void stub_VSM_Step(VSM_T* vsm)
{
    (void)vsm;
    mNumSteps++;
}

uint32_t mockGet_VSM_Step_Count(void)
{
    return mNumSteps;
}