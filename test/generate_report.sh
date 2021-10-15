#!/bin/bash
set -e

echo Running tests
# Run test to generate coverage files
find . -maxdepth 1 -regex "\.\/Test[a-zA-z]*" -exec {} \;

TEST_LIB_FILES=\
	coverage/mock* \
	coverage/Mock* \
	coverage/unity* \
	coverage/Test*

rm -f "$TEST_LIB_FILES"

echo Generating report
lcov -c -d ./coverage -o main_coverage.info
genhtml main_coverage.info --output-directory coverage_report
