all:
	cd test && make -j`nproc`
	./generate_report.sh

clean:
	# make -f test/Makefile clean
	rm -f Test*
	rm -f main_coverage.info
	rm -f *.gcda *.gcno
	rm -rf ./coverage
	rm -rf ./coverage_report
