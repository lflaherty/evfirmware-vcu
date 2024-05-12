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

# Clean up any local coverage files
find . -maxdepth 1 -name \*.gcno -exec rm {} \;
find . -maxdepth 1 -name \*.gcda -exec rm {} \;

echo Running tests
echo
TESTS="$(ls Test*)"

let counter=0 || true
summary=""

for TEST in ${TESTS}
do
    echo "Running ${TEST}"
    ./${TEST} -v
    echo

    let counter++ || true
done

echo "Successfully completed $counter tests"
echo

echo Stress testing
let counter=0 || true
summary=""

for TEST in ${TESTS}
do
    set +e # allow a failed test to be detected
    out="$(./${TEST} -v -r 25)"
    status=$?
    if [ ${status} -ne 0 ]; then
        echo "Failed 25 run ${TEST}"
        echo "${out}"
    fi

    set -e
    let counter++ || true
done

echo "Successfully completed $counter stress tests"
echo

# Copy all the coverage files here
find . -name \*.gcno -exec cp {} . \;
find . -name \*.gcda -exec cp {} . \;

echo Generating report
# Create report for all files
lcov --directory . -c -o main_coverage.info 2> /dev/null
# Only include directories requested
echo "${SRC_LIST[*]}" | xargs lcov -o main_coverage.info --extract main_coverage.info

# # Create report webpage
# rm -rf ../coverage_report
genhtml main_coverage.info --output-directory coverage_report > /dev/null
