#!/bin/bash

ROOT=$(dirname "$0")

# Create virtual serial port to run sim through
touch tty_sim tty_sim_internal
socat -d -d pty,raw,echo=0,link=tty_sim pty,raw,echo=0,link=tty_sim_internal &
pid_socat=$!

# Launch sim script using virtual serial port "tty_sim_internal"
python3 ${ROOT}/sim/sim-server.py

wait ${pid_socat}
