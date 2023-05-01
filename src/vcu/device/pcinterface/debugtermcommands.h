/*
 * debugtermcommands.h
 *
 * Interface for debug term command set.
 *
 *  Created on: 30 Apr 2023
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_PCINTERFACE_DEBUGTERMCOMMANDS_H_
#define DEVICE_PCINTERFACE_DEBUGTERMCOMMANDS_H_

#include "pcinterface.h"
#include <stdint.h>

#define DEBUGTERM_NUM_ARGS 8U

typedef void (*DebugTerm_CmdExec)(
    PCInterface_T* pcinterface,
    uint16_t argc,
    char argv[DEBUGTERM_NUM_ARGS][PCINTERFACE_DEBUGTERM_BUFLEN+1]);

struct DebugTerm_CmdDef {
  const char* name;
  const char* desc;
  const char* help;
  DebugTerm_CmdExec exec;
};

extern struct DebugTerm_CmdDef DebugTerm_Commands[];
extern const size_t DebugTerm_NumCommands;

#endif // DEVICE_PCINTERFACE_DEBUGTERMCOMMANDS_H_
