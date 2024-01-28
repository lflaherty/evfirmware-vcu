/*
 * debugterm.c
 *
 * Implemented serial console functionality.
 * 
 *  Created on: Apr 7 2023
 *      Author: Liam Flaherty
 */
#include "pcinterface.h"
#include "debugtermcommands.h"
#include "debugtermlib.h"

#include <stdint.h>
#include <string.h>
#include "comm/uart/uart.h"

/**
 * @brief Copies the src string to the dest string, up to a maximum length.
 * Stops when a terminating character \0 is found.
 *
 * @param dest Destination string.
 * @param src Source string.
 * @param maxLen Max buffer length
 * @return Number of characters copied
 */
static uint16_t strncpyl(char* dest, char* src, uint16_t maxLen)
{
  uint16_t i = 0;
  while (i < maxLen) {
    if (src[i] == '\0') {
      break;
    }
    dest[i] = src[i];
    ++i;
  }
  return i;
}

/**
 * @brief Attempt to invoke the processed string.
 * Ensure that argv[][] is 
 * 
 * @param pcinterface PCInterface struct
 * @param argc Number of args
 * @param argv Args - Array of strings
 */
static void processCommand(
    PCInterface_T* pcinterface,
    uint16_t argc,
    char argv[DEBUGTERM_NUM_ARGS][PCINTERFACE_DEBUGTERM_BUFLEN+1])
{
  // match argv[0] to the commands list
  for (size_t i = 0; i < DebugTerm_NumCommands; ++i) {
    struct DebugTerm_CmdDef* cmdDef = &DebugTerm_Commands[i];
    if (strcmp(argv[0], cmdDef->name) == 0) {
      cmdDef->exec(pcinterface, argc, argv);
    }
  }
}

static void processCommandStr(
    PCInterface_T* pcinterface,
    char* cmd,
    uint16_t cmdLen)
{
  // split into tokens
  char cmdArgs[DEBUGTERM_NUM_ARGS][PCINTERFACE_DEBUGTERM_BUFLEN+1] = { 0 };
  uint16_t nArgs = 0;

  uint16_t argIdx = 0;
  bool ws = false;

  for (uint16_t i = 0; i < cmdLen; ++i) {
    char c = cmd[i];

    // check for whitespace or end of command string (arg termination)
    if (c == ' ' || c == '\t') {
      // use 'ws' variable to avoid processing multiple spaces as arguments
      if (!ws) {
        // First time seeing ws, so end of arg
        nArgs++;
        argIdx = 0;
      }
      ws = true;
    } else if(c == '\n') {
      // final character of command
      nArgs++;
      argIdx = 0;
    } else {
      cmdArgs[nArgs][argIdx] = c;
      argIdx++;
      ws = false;
    }
  }

  processCommand(pcinterface, nArgs, cmdArgs);
}

static void processBuffer(PCInterface_T* pcinterface)
{
  struct PCInterface_DebugTerm* term = &pcinterface->debugterm;

  bool receivedFullCmd = false;
  uint16_t lastObservedBegin = 0;

  char cmdBuf[PCINTERFACE_DEBUGTERM_BUFLEN] = { 0 };
  uint16_t cmdBufIdx = 0;

  for (uint16_t i = 0; i < term->next; ++i) {
    cmdBuf[cmdBufIdx++] = term->buf[i];

    if (term->buf[i] == '\n') {
      // End of command
      receivedFullCmd = true;
      lastObservedBegin = i + 1;
      processCommandStr(pcinterface, cmdBuf, cmdBufIdx);
      continue;
    }
  }

  // buffer completed
  // shift any remaining bytes forward
  for (uint16_t i = lastObservedBegin; i < term->next; ++i) {
    term->buf[i - lastObservedBegin] = term->buf[i];
  }
  term->next -= lastObservedBegin;

  // New prompt for next command
  if (receivedFullCmd) {
    DebugPrint(pcinterface, "> ");
  }
}


void PCInterface_DebugTermRecv(
    PCInterface_T* pcinterface,
    uint8_t* payloadBytes,
    uint16_t nBytes)
{
  if (pcinterface->debugterm.next + nBytes > PCINTERFACE_DEBUGTERM_BUFLEN) {
    // something's gone wrong and there is too much data for the buffer
    // clear buffer and drop message
    DebugPrint(pcinterface, "error: buffer full\n");
    pcinterface->debugterm.next = 0;
    return;
  }

  struct PCInterface_DebugTerm* term = &pcinterface->debugterm;
  uint16_t nCopied = strncpyl(term->buf + term->next, (char*)payloadBytes, nBytes);
  term->next += nCopied;

  if (0 == term->next) {
    return;
  }

  processBuffer(pcinterface);
}
