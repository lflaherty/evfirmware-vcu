/*
 * messages.h
 *
 * Defines details of UART debug protocol messages
 *
 *  Created on: Dec 2 2022
 *      Author: Liam Flaherty
 */

#ifndef DEVICE_PCINTERFACE_MESSAGES_H_
#define DEVICE_PCINTERFACE_MESSAGES_H_

#define PCINTERFACE_MSG_DESTADDR_VCU    0x01
#define PCINTERFACE_MSG_DESTADDR_PC     0x02

// Number of non-payload bytes in the packet
#define PCINTERFACE_MSG_PACKET_BYTES    11

// Tx messages:

#define PCINTERFACE_MSG_STATEUPDATE_FUNCITION 0x01
#define PCINTERFACE_MSG_STATEUPDATE_DATALEN   7U
#define PCINTERFACE_MSG_STATEUPDATE_MSGLEN    18U

#define PCINTERFACE_MSG_LOG_FUNCTION 0x02
#define PCINTERFACE_MSG_LOG_DATALEN  32U
#define PCINTERFACE_MSG_LOG_MSGLEN   43U

// Variable length message
#define PCINTERFACE_MSG_DEBUGTERM_FUNCTION 0x09
#define PCINTERFACE_MSG_DEBUGTERM_BUFFERLEN 256

// Rx messages:

// Use a common message length to simplifying decoding
#define PCINTERFACE_MSG_COMMON_DATALEN  8U
#define PCINTERFACE_MSG_COMMON_MSGLEN   19U

#define PCINTERFACE_MSG_DEBUG_TERMINAL          0x9
#define PCINTERFACE_MSG_TESTCMD_SDC_FUNCTION    0x101
#define PCINTERFACE_MSG_TESTCMD_PDM_FUNCTION    0x102


#endif // DEVICE_PCINTERFACE_MESSAGES_H_
