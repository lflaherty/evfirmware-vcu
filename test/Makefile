all:
	cd test && make
	./generate_report.sh

clean:
	# make -f test/Makefile clean
	rm -f TestCan
	rm -f main_coverage.info
	rm -f *.gcda *.gcno
	rm -rf ./coverage
	rm -rf ./coverage_report
