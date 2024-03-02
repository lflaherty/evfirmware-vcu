#!/usr/bin/env python3
"""
Prints log data to terminal.
"""

import os
from time import sleep
from argparse import ArgumentParser
import serial

from lib.decode_common import MsgDecoder, MsgType
from lib.serial_common import SerialHandler
from lib.colors import bcolors

BAUD_RATE = 115200
STOP_BITS = serial.STOPBITS_ONE

ADDR_PC = 0x02
MSG_TYPE_LOG = 0x02
MSG_LEN_LOG = 43
MSG_TYPE_STATE = 0x01
MSG_LEN_STATE = 18

FIELD_ID_NAMES = {
  0x0001: 'SDC',
  0x0002: 'PDM',
  0x0003: 'BMS Max cell voltage',
  0x0004: 'BMS Max cell voltage ID',
  0x0005: 'BMS Max cell temp',
  0x0006: 'BMS Max cell temp ID',
  0x0007: 'BMS DC current',
  0x0008: 'BMS Pack voltage',
  0x0009: 'BMS SOC',
  0x000A: 'BMS Counter',
  0x000B: 'BMS Fault',
}

OPT_RAW = False

"""
Handler method for a log message data
"""
def handle_log_data(msg_info):
  msg_data = msg_info['payload']
  crc_correct = msg_info['crc_correct']
  msg_str = ''.join([chr(x) for x in msg_data])

  global OPT_RAW
  if OPT_RAW:
    msg_str = repr(msg_str)
  
  if crc_correct:
    print(f'{msg_str}', end='', flush=True)
  else:
    print(f'{bcolors.FAIL}{msg_str}{bcolors.ENDC}', end='', flush=True)


"""
Handler method for state message data
"""
def handle_state_data(msg_info):
  msg_data = msg_info['payload']
  crc_correct = msg_info['crc_correct']
  if crc_correct:
    field_id = (msg_data[0] << 8) | msg_data[1]
    field_len = msg_data[2]
    value = 0
    for i in range(field_len):
      ith_byte = msg_data[6 - i]
      value |= ith_byte << (i*8)

    if field_id not in FIELD_ID_NAMES:
      print('Unexpected field ID {}'.format(hex(field_id)))
      return

    print('State Update {} {}'.format(FIELD_ID_NAMES[field_id], hex(value)))


def main():
  parser = ArgumentParser()
  parser.add_argument('port', help='Serial port to open')
  parser.add_argument('-v', '--verbose', dest='verbose', action='store_true',
                      help='Verbose decoding logging')
  parser.add_argument('-r', '--raw', action='store_true',
                      help='View output data without formatting')
  parser.add_argument('-b', '--bytes', dest='bytes', action='store_true',
                      help='Print each byte as it is read')
  args = parser.parse_args()
  
  if args.raw:
    global OPT_RAW
    OPT_RAW = True

  print(f'{bcolors.HEADER}Launching threads{bcolors.ENDC}')

  msg_log_decoder = MsgDecoder(
    self_address=ADDR_PC,
    msg_type=MsgType(
      name="LOG",
      type=MSG_TYPE_LOG,
      len=MSG_LEN_LOG,
      handler=handle_log_data,
    ),
    print_bytes=args.bytes,
    verbose=args.verbose
  )
  msg_state_decoder = MsgDecoder(
    self_address=ADDR_PC,
    msg_type=MsgType(
      name="STATE",
      type=MSG_TYPE_STATE,
      len=MSG_LEN_STATE,
      handler=handle_state_data,
    ),
    print_bytes=args.bytes,
    verbose=args.verbose
  )

  serial_handler = SerialHandler(
      port=args.port,
      baud_rate=BAUD_RATE,
      stop_bits=STOP_BITS,
  )
  serial_handler.add_decoder(msg_log_decoder)
  serial_handler.add_decoder(msg_state_decoder)

  try:
    serial_handler.start()

    if os.name != 'nt':
      # Don't join serial_handler on windows - this would block ctrl-C
      serial_handler.join()
  except KeyboardInterrupt:
    print()
    print(f'{bcolors.HEADER}Quitting{bcolors.ENDC}')

  serial_handler.close_port()


if __name__ == '__main__':
  main()
