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
CFLAGS += -Werror
# Profiling flags
CFLAGS += -g
CFLAGS += -fprofile-arcs
CFLAGS += -ftest-coverage
CFLAGS += -fprofile-dir=./coverage
# Linker flags
LFLAGS=-lm

# Defines TEST_SRC, SRC_MOCKS, and COMMON_MOCKS
include test_files.mk

SRC_FILES=\
	Unity/src/unity.c \
	Unity/extras/fixture/src/unity_fixture.c \
	$(COMMON_MOCKS) \
	$(SRC_MOCKS) \
	$(TEST_SRC) \
	test/run_tests.c \

INC_DIRS=-I../src
INC_DIRS+=-Itest
INC_DIRS+=-Imock
INC_DIRS+=-IUnity/src
INC_DIRS+=-IUnity/extras/fixture/src
SYMBOLS=-DUNITY_FIXTURE_NO_EXTRAS

all_tests:
	@echo Build...
	$(GCC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) $(LFLAGS) -o run_tests
	./generate_report.sh ../src

clean:
	# make -f test/Makefile clean
	rm -f run_tests
	rm -f main_coverage.info
	rm -f *.gcda *.gcno
	rm -rf run_tests.dSYM
	rm -rf ./coverage
	rm -rf ./coverage_report
