/*
 * depends.c
 *
 *  Created on: 9 Nov 2022
 *      Author: Liam Flaherty
 */

#include "depends.h"
#include <stdio.h>

// Start at 1 to reserve 0 as NULL
static uint32_t nModules = 1U;

bool RegisterModule(CompRegID_T* newCompId)
{
  if (NULL == newCompId) {
    return false;
  }

  *newCompId = nModules++;

  return true;
}

bool DependOn(CompRegID_T compId)
{
  if (compId > 0 && compId < nModules) {
    // We can't de-register components so if it was registered, it will
    // have an ID within this range.
    return true;
  }

  return false;
}
