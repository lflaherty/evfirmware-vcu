# ECU Config Tool

This tool is the client for the ECU-PC RS232 interface.

## Tools

 * `log_view.py` Opens a serial port, decodes messages with the log message ID, and prints content to stdout.
 * `sim-launch.sh` Local config sim. See below.

## Sim

To test the client tool without an ECU conection, the sim can be used. The sim will create a virtual serial port exposed at the file `tty_sim`.

To run the sim, simply:

1. In one terminal: `sim-launch.sh`
2. In a second terminal: `log_view.py tty_sim`

## Files

| File | Description |
| ---- | ----------- |
| log_view.py | Decodes log and state update messages |
| sim-launch.sh | Starts ECU config tool sim server |
| lib/colors.py | enum of colors for terminal |
| lib/decode_common.py | Logic for decoding messages sent from ECU |
| lib/encode_common.py | Logic for encoding messages to send to ECU |
| lib/serial_common.py | Abstraction for communicating to serial port |
| sim/sim-server.py | Sim server program to communicate over virtual serial port |
