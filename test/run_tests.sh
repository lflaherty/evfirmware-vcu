#!/bin/bash -e

mkdir -p build
cd build

# Compile
cmake ..
make -j7

# Generate report
#../generate_report.sh ../../Application ../../System
../generate_report.sh ../../src

