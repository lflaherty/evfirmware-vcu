/*
 * debugtermcommands.c
 *
 * Interface for debug term command set.
 *
 *  Created on: 30 Apr 2023
 *      Author: Liam Flaherty
 */
#include "debugtermcommands.h"
#include "debugtermlib.h"

#include <stdio.h>

/**
 * @brief Search the command list for a command with a specific name.
 * 
 * @param name 
 * @return struct DebugTerm_CmdDef* 
 */
static struct DebugTerm_CmdDef* findCmdByName(const char* name)
{
  for (size_t i = 0; i < DebugTerm_NumCommands; ++i) {
    struct DebugTerm_CmdDef* cmd = &DebugTerm_Commands[i];
    
    if (strcmp(cmd->name, name) == 0) {
      return cmd;
    }
  }

  return NULL;
}

static void cmd_help(
    PCInterface_T* pcinterface,
    uint16_t argc,
    char argv[DEBUGTERM_NUM_ARGS][PCINTERFACE_DEBUGTERM_BUFLEN+1])
{
  switch (argc) {
    case 1: { // No args given, just print list of commands
      DebugPrint(pcinterface, "Available commands:\n");
      for (size_t i = 0; i < DebugTerm_NumCommands; ++i) {
        struct DebugTerm_CmdDef* cmd = &DebugTerm_Commands[i];

        char helpLineBuf[DEBUGTERM_MAX_MSG_LEN] = { 0 };
        snprintf(helpLineBuf, DEBUGTERM_MAX_MSG_LEN, "%-12s %s\n",
            cmd->name, cmd->desc);

        DebugPrint(pcinterface, helpLineBuf);
      }
      break;
    }
    case 2: { // Print help string for the given arg
      char* cmdName = argv[1];
      struct DebugTerm_CmdDef* cmd = findCmdByName(cmdName);
      DebugPrint(pcinterface, cmd->desc);
      DebugPrint(pcinterface, "\n");
      DebugPrint(pcinterface, cmd->help);
      break;
    }
    default: { // unexpected number of args given
      const char* defaultString = "Unexpected number of arguments.\n\n";
      DebugPrint(pcinterface, defaultString);

      // Print normal help command
      char simpleHelpArgv[DEBUGTERM_NUM_ARGS][PCINTERFACE_DEBUGTERM_BUFLEN+1];
      memset(simpleHelpArgv, 0, sizeof(simpleHelpArgv));
      snprintf(simpleHelpArgv[0], PCINTERFACE_DEBUGTERM_BUFLEN, "help");
      cmd_help(pcinterface, 1, simpleHelpArgv);
    }
  }
}

static void cmd_setpdm(
    PCInterface_T* pcinterface,
    uint16_t argc,
    char argv[DEBUGTERM_NUM_ARGS][PCINTERFACE_DEBUGTERM_BUFLEN+1])
{
  if (argc != 7) {
    DebugPrint(pcinterface, "Unexpected number of params\n");
    return;
  }

  bool pdmReqeusts[6] = { false };
  for (uint16_t i = 0; i < 6; ++i) {
    char c = argv[i+1][0];
    if (c == '0') {
      pdmReqeusts[i] = false;
    } else if (c == '1') {
      pdmReqeusts[i] = true;
    } else {
      DebugPrint(pcinterface, "Param must be 0 or 1\n");
      return;
    }
  }

  DebugPrint(pcinterface, "Test command executed: set PDM states to\n");
  for (uint8_t i = 0U; i < 6; ++i) {
    VehicleControl_SetPowerChannel(pcinterface->control, i, pdmReqeusts[i]);

    char buf[32] = { 0 };
    snprintf(buf, 32, "  PDM channel %u: %u\n", i, pdmReqeusts[i]);
    DebugPrint(pcinterface, buf);
  }
}

static void cmd_setsdc(
    PCInterface_T* pcinterface,
    uint16_t argc,
    char argv[DEBUGTERM_NUM_ARGS][PCINTERFACE_DEBUGTERM_BUFLEN+1])
{
  if (argc != 2) {
    DebugPrint(pcinterface, "Unexpected number of params\n");
    return;
  }

  bool sdcAssert = false;
  char sdcAssertChar = argv[1][0];
  switch (sdcAssertChar) {
    case '0':
      sdcAssert = false;
      break;
    case '1':
      sdcAssert = true;
      break;
    default:
      DebugPrint(pcinterface, "Param must be 0 or 1\n");
      return;
  }

  VehicleControl_SetECUError(pcinterface->control, sdcAssert);

  char buf[32] = { 0 };
  snprintf(buf, 32, "  SDC output set to: %u\n", sdcAssert);
  DebugPrint(pcinterface, buf);
}

struct DebugTerm_CmdDef DebugTerm_Commands[] = {
  {
    .name = "help",
    .desc = "Display command help screen",
    .help = "Use help [cmd] for extended help for a specific command",
    .exec = cmd_help,
  },
  {
    .name = "setpdm",
    .desc = "Control PDM channels",
    .help = "Usage: setpdm 1 2 3 4 5 6\n"
            "1-6 are the on off states for channels 1-6, respectively.\n"
            "For example, `setpdm 0 0 1 0 1 0` turns on channels 3 and 5, and switches\n"
            "off remaining channels.\n",
    .exec = cmd_setpdm,
  },
  {
    .name = "setsdc",
    .desc = "Control shutdown circuit output state",
    .help = "Usage: setsdc x\n"
            "where x is 0 or 1 to disable or enable the SDC output.\n",
    .exec = cmd_setsdc,
  },
};
const size_t DebugTerm_NumCommands = sizeof(DebugTerm_Commands) / sizeof(DebugTerm_Commands[0]);
