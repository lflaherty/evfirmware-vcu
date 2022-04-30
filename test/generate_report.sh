#!/bin/bash
set -e

if [ -z $1 ]; then
    echo "Usage: generate_report.sh path_to_path"
    exit 1
fi
src_path=$1
full_src_path="$(cd ${src_path} && pwd)"

# Reset counters
lcov --directory . --zerocounters -q

echo Running tests
./run_tests
echo

echo Generating report
# Create report for all files
lcov --directory . -c -o main_coverage.info
# Only include ../src directory (i.e. exlucde test and mock helper code)
lcov --extract main_coverage.info "${full_src_path}*" -o main_coverage.info

# Create report webpage
rm -rf ../coverage_report
genhtml main_coverage.info --output-directory coverage_report
