#!/bin/bash
set -e

if [ -z $1 ]; then
    echo "Usage: generate_report.sh path_to_src1 ..."
    exit 1
fi

echo "Reading paths"
SRC_LIST=()
for src_path in "$@"
do
    if [ -d "$src_path" ]; then
        full_src_path="$(cd ${src_path} && pwd)"
        SRC_LIST+=("${full_src_path}*")
        echo "$src_path -> $full_src_path"
    else
        echo "$src_path is not a directory"
        exit 1
    fi
done

# Reset counters
lcov --directory . --zerocounters -q

echo Running tests
./run_tests -v
echo

echo Generating report
# Create report for all files
lcov --directory . -c -o main_coverage.info
# Only include directories requested
echo "${SRC_LIST[*]}" | xargs lcov -o main_coverage.info --extract main_coverage.info

# Create report webpage
rm -rf ../coverage_report
genhtml main_coverage.info --output-directory coverage_report
