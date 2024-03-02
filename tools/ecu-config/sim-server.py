#!/usr/bin/env python3

import serial
import time
from encode_common import encode_message

PORT = 'tty_sim_internal'
BAUD_RATE = 115200
STOP_BITS = serial.STOPBITS_ONE

ADDR_PC = 0x02
MSG_TYPE_LOG = 0x02
MSG_LEN_LOG = 43


def create_log_msgs(str):
    msgs = []
    LOG_PAYLOAD_LEN = 32
    for i in range(0, len(str), LOG_PAYLOAD_LEN):
        payload = [0x00] * LOG_PAYLOAD_LEN
        for j,c in enumerate(str[i:i+LOG_PAYLOAD_LEN]):
            payload[j] = ord(c)

        msg = encode_message(ADDR_PC, MSG_TYPE_LOG, payload)
        msgs.append(msg)
    return msgs


if __name__ == '__main__':
    print('Opening serial port tty_sim_internal')
    serial = serial.Serial(PORT, BAUD_RATE, stopbits=STOP_BITS, timeout=0.5)
    counter = 0
    while True:
        counter += 1
        send_str = f'Test log message {counter}\n'
        print(f'Sending string: {send_str}')
        msgs = create_log_msgs(send_str)
        for msg in msgs:
            serial.write(msg)
        time.sleep(1)
