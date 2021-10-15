#!/bin/bash
set -e

echo Running tests
# Run test to generate coverage files
for file in `find . -maxdepth 1 -regex "\.\/Test[a-zA-z]*"`
do
	echo "Running $file"
	./$file
done

# Remove files not to include in the coverage report
rm -f coverage/mock*
rm -f coverage/Mock*
rm -f coverage/unity*
rm -f coverage/Test*

# echo Generating report
lcov -c -d ./coverage -o main_coverage.info
genhtml main_coverage.info --output-directory coverage_report
