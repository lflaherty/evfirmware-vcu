#!/bin/bash -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd -P)"
TEST_DIR=${SCRIPT_DIR}
SRC_DIR=${SCRIPT_DIR}/../src
BUILD_DIR=${SCRIPT_DIR}/build

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

cmake ${TEST_DIR}
make -j$(nproc)

${TEST_DIR}/generate_report.sh ${SRC_DIR}
