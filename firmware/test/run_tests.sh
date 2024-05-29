#!/bin/bash -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd -P)"
TEST_DIR=${SCRIPT_DIR}
SRC_DIR=${SCRIPT_DIR}/../src
BUILD_DIR=${SCRIPT_DIR}/build

mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}

# The report generation in it's current form supports clang coverage
# reporting, and not gcov...
# Compiling and running tests still works with gcc

cmake  -DCMAKE_C_COMPILER=clang ${TEST_DIR}
make -j$(nproc)

${TEST_DIR}/generate_report.sh ${SRC_DIR}
