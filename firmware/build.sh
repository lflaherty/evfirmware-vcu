#!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd -P)"
SRC_DIR=${SCRIPT_DIR}/src
BUILD_DIR=${SCRIPT_DIR}/build

echo $SCRIPT_DIR

TOOLCHAIN_DIR=${SCRIPT_DIR}/third_party/cmake-arm-embedded
TOOLCHAIN_SCRIPT=${TOOLCHAIN_SCRIPT}/toolchain-arm-none-eabi.cmake

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

echo "building ${SRC_DIR}"
cmake -DCMAKE_TOOLCHAIN_FILE=${TOOLCHAIN_SCRIPT} ${SRC_DIR}
make -j$(nproc)
