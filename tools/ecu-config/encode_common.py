from decode_common import crc32mpeg2

CHAR_COLON = 0x3A
CHAR_CR = 0x0D
CHAR_LF = 0x0A

"""
Returns byte array of standard message given input params

Args:
    target_addr: integer address of receiving device
    function: integer message function/type
    payload: standard array of message payload bytes
"""
def encode_message(
    target_addr,
    function,
    payload
):
    msg_header = [
        CHAR_COLON,
        # target address
        (target_addr >> 8) & 0xFF,
        target_addr & 0xFF,
        # message function
        (function >> 8) & 0xFF,
        function & 0xFF
    ]
    msg_trailing = [
        0, 0, 0, 0, # CRC placeholder
        CHAR_CR, CHAR_LF
    ]

    # Calculate initial message without zeroed CRC
    msg = msg_header + payload + msg_trailing
    calc_crc = crc32mpeg2(msg)

    # Re-construct message with CRC
    msg_trailing = [
        (calc_crc >> 24) & 0xFF,
        (calc_crc >> 16) & 0xFF,
        (calc_crc >> 8) & 0xFF,
        (calc_crc >> 0) & 0xFF,
        CHAR_CR, CHAR_LF
    ]
    msg = msg_header + payload + msg_trailing

    return bytes(msg)