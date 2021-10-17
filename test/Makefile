all:
	@echo "Building evfirmware-lib System tests"
	cd ../System/test/test && make -j`nproc` BUILD_DIR_CD=../../test LIB_TEST_DIR=../System/test
	
	@echo "Building VCU tests"
	cd test && make -j`nproc`
	
	@echo "Generating coverage report"
	../System/test/generate_report.sh

clean:
	# make -f test/Makefile clean
	rm -f Test*
	rm -f main_coverage.info
	rm -f *.gcda *.gcno
	rm -rf ./coverage
	rm -rf ./coverage_report
