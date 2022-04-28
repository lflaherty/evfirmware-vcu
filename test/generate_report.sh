#!/bin/bash
set -e

# Reset counters
lcov --directory . --zerocounters -q

echo Running tests
./run_tests
echo

echo Generating report
# Create report for all files
lcov --directory . -c -o main_coverage.info
# Only include ../src directory (i.e. exlucde test and mock helper code)
src_path="$(cd ../src && pwd)"
lcov --extract main_coverage.info "${src_path}*" -o main_coverage.info

# Create report webpage
rm -rf ../coverage_report
genhtml main_coverage.info --output-directory coverage_report
