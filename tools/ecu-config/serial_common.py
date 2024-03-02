from threading import Thread
import serial

from colors import bcolors

class SerialHandler:
    def __init__(
        self,
        port,
        baud_rate,
        stop_bits
    ):
        self.decoders = []
        self.thread_recv = Thread(target=self.recv, daemon=True)
        self.open_port(
            port=port,
            baud_rate=baud_rate,
            stop_bits=stop_bits
        )

    def open_port(
        self,
        port,
        baud_rate,
        stop_bits,
    ):
        print(f'{bcolors.HEADER}Opening port {port}{bcolors.ENDC}')
        self.serial = serial.Serial(port, baud_rate, stopbits=stop_bits, timeout=0.5)

    def close_port(self):
        print(f'{bcolors.HEADER}Closing serial{bcolors.ENDC}')
        self.serial.close()

    def recv(self):
        print(f'{bcolors.HEADER}Listening for data{bcolors.ENDC}')
        print()

        while True:
            # Receive new bytes
            b = self.serial.read(10)
            if not b:
                continue

            for decoder in self.decoders:
                decoder.recv_bytes(b)

    def start(self):
        self.thread_recv.start()

    def join(self):
        self.thread_recv.join()

    def add_decoder(self, decoder):
        self.decoders.append(decoder)


class SerialTx:
    def __init__(
        self,
        serial_handler,
        send_handler = None,
    ):
        self.serial_handler = serial_handler
        self.send_handler = send_handler
        self.thread = Thread(target=self.send, daemon=True)
    
    def start(self):
        self.thread.start()
    
    def write_bytes(self, array):
        self.serial_handler.serial.write(array)
    
    def send(self):
        while True:
            x = input()
            if self.send_handler:
                self.send_handler(x)
            else:
                print(f'{bcolors.HEADER}Sending', repr(x), f'{bcolors.ENDC}')
                self.write_bytes(x.encode('utf-8'))
    
    def join(self):
        self.thread.join()
