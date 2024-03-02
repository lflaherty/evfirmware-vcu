from threading import Thread

from colors import bcolors

# Constants
CHAR_COLON = 0x3A
CHAR_CR = 0x0D
CHAR_LF = 0x0A

MIN_NUM_BYTES = 10
MAX_BUFFER_LEN = 1024

"""
Use the MPEG-2 variant of CRC, because that's what the STM32 uses.
"""
def crc32mpeg2(buf, crc=0xffffffff):
    for val in buf:
        crc ^= val << 24
        for _ in range(8):
            crc = crc << 1 if (crc & 0x80000000) == 0 else (crc << 1) ^ 0x104c11db7
    return crc


MSG_TYPE_LEN_VARIABLE = None

class MsgType:
    """
    Set len to None for variable length support
    """
    def __init__(
        self,
        name,
        type,
        len,
        handler,
    ):
        self.name = name
        self.type = type
        self.len = len
        self.handler = handler


class MsgDecoder:
    def __init__(
        self,
        self_address, # address that we expect to receive from
        msg_type: MsgType,
        print_bytes = False,
        verbose = False,
    ):
        self.OPT_PRINT_BYTES = print_bytes
        self.OPT_VERBOSE = verbose
        self.self_address = self_address
        self.msg_buffer = []
        # array of MsgTypes
        self.message_type = msg_type

    def recv_bytes(self, b):
        # can receive multiple bytes at once
        n = len(b)
        b = int.from_bytes(b, 'little')
        for i in range(n):
            current_byte = (b >> (i*8)) & 0xFF
            self.msg_buffer.append(current_byte)

            if self.OPT_PRINT_BYTES:
                print(f'Received "{bcolors.OKCYAN}', hex(current_byte), f'{bcolors.ENDC}"')

        # Should always start with a ':' character, so pop all bytes that aren't that
        while len(self.msg_buffer) > 0 and self.msg_buffer[0] != CHAR_COLON:
            self.msg_buffer.pop(0)

        if self.OPT_VERBOSE:
            print(f'{bcolors.OKBLUE}Buffer contents:', self.msg_buffer, '({})'.format(len(self.msg_buffer)), f'{bcolors.ENDC}')

        # Wait until we have enough bytes
        if self.message_type.len == MSG_TYPE_LEN_VARIABLE:
            self.decode_bytes_variable_len(b)
        else:
            self.decode_bytes_fixed_len(b)

    def decode_bytes_fixed_len(self, b):
        while len(self.msg_buffer) >= self.message_type.len:
            if self.OPT_VERBOSE:
                print(f'{bcolors.OKBLUE}Attempting decode{bcolors.ENDC}')

            try_length = self.message_type.len
            msg_received = self.try_decode(self.msg_buffer[:try_length], try_length)
            if msg_received:
                if self.OPT_VERBOSE:
                    print(f'{bcolors.OKBLUE}Found valid message {self.message_type.name}{bcolors.ENDC}')

                self.handle_data(msg_received)

                self.msg_buffer = self.msg_buffer[try_length:]
            else:
                self.msg_buffer = self.msg_buffer[1:]

    def decode_bytes_variable_len(self, b):
        if len(self.msg_buffer) < MIN_NUM_BYTES:
            return

        if self.OPT_VERBOSE:
            print(f'{bcolors.OKBLUE}Attempting decode{bcolors.ENDC}')

        # print(len(self.msg_buffer))
        # print('loop attempts:', (len(self.msg_buffer) - MIN_NUM_BYTES)*(len(self.msg_buffer)-MIN_NUM_BYTES))
        for try_offset in range(0, len(self.msg_buffer) - MIN_NUM_BYTES):
            # The beginning of the msg could be offset slightly into the buffer
            # so try multiple offsets into buffer
            for try_len in range(MIN_NUM_BYTES, len(self.msg_buffer)):
                # The msg could be any length, so try all options until something valid
                # is received.
                # We rely on the CRC being correct too, so even if a CRC failure
                # may be allowed (e.g. on log messages), the message will still
                # be lost.

                msg_attempt = self.msg_buffer[try_offset:try_offset+try_len]

                msg_valid = False
                msg_received = self.try_decode(msg_attempt, try_len)
                if msg_received:
                    if msg_received['crc_correct']:
                        msg_valid = True
                        if self.OPT_VERBOSE:
                            print(f'{bcolors.OKBLUE}Found valid message {self.message_type.name}{bcolors.ENDC}')
                    else:
                        if self.OPT_VERBOSE:
                            print(f'{bcolors.OKBLUE}CRC mismatch on decode attempt {self.message_type.name}{bcolors.ENDC}')

                if msg_valid:
                    self.handle_data(msg_received)
                    self.msg_buffer = self.msg_buffer[try_offset+try_len:]

    """
    Returns None if no msg received
    Returns dict summarizing received data.

    Params:
        msg: array buffer to attempt to decode
        try_type: MsgType object describing message type details to attempt
    """
    def try_decode(self, msg, expected_len):
        if self.OPT_VERBOSE:
            print(f'{bcolors.OKBLUE}try_decode on {self.message_type.name}{bcolors.ENDC}')
        if len(msg) != expected_len:
            return None

        expected_bytes = [
            msg[0] == CHAR_COLON,
            msg[-2] == CHAR_CR,
            msg[-1] == CHAR_LF,
        ]
        if not all(expected_bytes):
            return None

        # Get CRC from message
        msg_crc = 0
        offset = 24
        for byte in msg[-6:-2]:
            msg_crc = msg_crc | (byte << offset)
            offset -= 8

        msg[-6:-2] = 4*[0]

        calc_crc = crc32mpeg2(msg)
        if self.OPT_VERBOSE:
            print(f'{bcolors.OKBLUE}Calculated CRC', hex(calc_crc),
                'got', hex(msg_crc), f'{bcolors.ENDC}')

        crc_correct = calc_crc == msg_crc
        if not crc_correct and self.OPT_VERBOSE:
            print(f'{bcolors.OKBLUE}CRC mismatch. Expecting', hex(calc_crc),
                'got', hex(msg_crc), f'{bcolors.ENDC}')

        addr = (msg[1] << 8) | msg[2]
        msg_type = (msg[3] << 8) | msg[4]
        recv_data = msg[5:-6]

        ret = {
            'payload': recv_data,
            'msg_type': msg_type,
            'addr': addr,
            'crc_calc': calc_crc,
            'crc_recv': msg_crc,
            'crc_correct': crc_correct
        }
        return ret

    """
    Handle the contents of the valid data message
        msg: result of try_decode
        msg_type: value from self.message_types for this message ID
    """
    def handle_data(self, msg):
        if msg['addr'] != self.self_address:
            if self.OPT_VERBOSE:
                addr = msg['addr']
                print(f'{bcolors.OKBLUE}Unexpected address {addr}{bcolors.ENDC}')
            return
        if msg['msg_type'] != self.message_type.type:
            if self.OPT_VERBOSE:
                type = msg['msg_type']
                print(f'{bcolors.OKBLUE}Unexpected type {type}{bcolors.ENDC}')
            return

        if self.OPT_VERBOSE:
            print(f'{bcolors.OKBLUE}Invoking message handler{bcolors.ENDC}')

        self.message_type.handler(msg)
