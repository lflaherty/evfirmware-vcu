GCC=gcc
# Compile flags
CFLAGS=-std=c99
# Warning flags
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
CFLAGS += -Wold-style-definition
CFLAGS += -Wdouble-promotion
CFLAGS += -Wshadow
CFLAGS += -Wformat=2
CFLAGS += -Wconversion
CFLAGS += -Werror
# Profiling flags
CFLAGS += -g
CFLAGS += -fprofile-arcs
CFLAGS += -fstack-usage
CFLAGS += -ftest-coverage
CFLAGS += -fprofile-dir=./coverage
# Sanitizers
CFLAGS += -fsanitize=address
CFLAGS += -fsanitize=pointer-compare
CFLAGS += -fsanitize=pointer-subtract
CFLAGS += -fsanitize=undefined
# Linker flags
LFLAGS=-lm

# Defines TEST_SRC, SRC_MOCKS, and COMMON_MOCKS
include test_files.mk

SRC_FILES=\
	systemlib-test/Unity/src/unity.c \
	systemlib-test/Unity/extras/fixture/src/unity_fixture.c \
	$(COMMON_MOCKS) \
	$(SRC_MOCKS) \
	$(TEST_SRC) \
	test/run_tests.c \

INC_DIRS=-I../System/src
INC_DIRS+=-I../Application
INC_DIRS+=-Imock/Application
INC_DIRS+=-Isystemlib-test/test
INC_DIRS+=-Isystemlib-test/mock
INC_DIRS+=-Isystemlib-test/Unity/src
INC_DIRS+=-Isystemlib-test/Unity/extras/fixture/src
SYMBOLS=-DUNITY_FIXTURE_NO_EXTRAS

all_tests:
	@echo Build...
	$(GCC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) $(LFLAGS) -o run_tests
	./systemlib-test/generate_report.sh ../System/src ../Application

clean:
	# make -f test/Makefile clean
	rm -f run_tests
	rm -f main_coverage.info
	rm -f *.gcda *.gcno
	rm -rf run_tests.dSYM
	rm -rf ./coverage
	rm -rf ./coverage_report
