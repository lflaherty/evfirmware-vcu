#!/usr/bin/env python3

import serial
import time

PORT = 'tty_sim_internal'
BAUD_RATE = 115200
STOP_BITS = serial.STOPBITS_ONE

START_BYTE = 0x3A
CHAR_CR = 0x0D
CHAR_LF = 0x0A

ADDR_PC = 0x02
MSG_TYPE_LOG = 0x02
MSG_LEN_LOG = 43


"""
Use the MPEG-2 variant of CRC, because that's what the STM32 uses.
"""
def crc32mpeg2(buf, crc=0xffffffff):
    for val in buf:
        crc ^= val << 24
        for _ in range(8):
            crc = crc << 1 if (crc & 0x80000000) == 0 else (crc << 1) ^ 0x104c11db7
    return crc



def create_log_msgs(str):
    msgs = []
    LOG_PAYLOAD_LEN = 32
    for i in range(0, len(str), LOG_PAYLOAD_LEN):
        header = [START_BYTE, 0x00, ADDR_PC, 0x00, MSG_TYPE_LOG]
        tail = [0x00, 0x00, 0x00, 0x00, CHAR_CR, CHAR_LF]  # 4 bytes for CRC
        payload = [0x00] * LOG_PAYLOAD_LEN
        for j,c in enumerate(str[i:i+LOG_PAYLOAD_LEN]):
            payload[j] = ord(c)

        msg = header + payload + tail

        crc = crc32mpeg2(msg)
        msg[-6] = (crc >> 24) & 0xFF
        msg[-5] = (crc >> 16) & 0xFF
        msg[-4] = (crc >> 8) & 0xFF
        msg[-3] = (crc >> 0) & 0xFF

        msgs.append(bytes(msg))
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
