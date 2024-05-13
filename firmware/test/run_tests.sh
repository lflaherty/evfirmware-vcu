#!/bin/bash -e

mkdir -p build
pushd build

# Compile
cmake ..
make -j$(nproc)

# Generate report
../generate_report.sh ../../src

popd
