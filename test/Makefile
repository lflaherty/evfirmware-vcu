GCC=gcc
CFLAGS=-std=c99
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
CFLAGS += -g -fprofile-arcs -ftest-coverage

UNITY_ROOT=./Unity

TARGET=all_tests
MOCKS=\
	mock/FreeRTOS/mockFreeRTOS.c \
	mock/FreeRTOS/mockQueue.c \
	mock/FreeRTOS/mockTask.c \
	mock/stm32_hal/MockStm32f7xx_hal.c \
	mock/stm32_hal/MockStm32f7xx_hal_can.c \
	mock/MockLogging.c

TESTED_FILES=\
	../src/comm/can/can.c \
	../src/lib/logging/logging.c

TEST_FILES=\
	test/lib/logging/TestLogMock.c \
	test/comm/can/TestCan.c

SRC_FILES=\
	$(UNITY_ROOT)/src/unity.c \
	$(UNITY_ROOT)/extras/fixture/src/unity_fixture.c \
	$(MOCKS) \
	$(TESTED_FILES) \
	$(TEST_FILES) \
	test/all_tests.c
	

INC_DIRS=-I../src -Itest -Imock -I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src
SYMBOLS=-DUNITY_FIXTURE_NO_EXTRAS

all: clean default coverage

default:
	@echo Build...
	$(GCC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) -o $(TARGET)

coverage:
	@echo Coverage...
	./$(TARGET)

	# without paths: $(notdir $(TESTED_FILES))
	# replacing paths with .gcno $(patsubst %.c,%,$(notdir $(TESTED_FILES)))

	mkdir -p coverage
	mv $(patsubst %.c,%.gcno,$(notdir $(TESTED_FILES))) coverage
	mv $(patsubst %.c,%.gcda,$(notdir $(TESTED_FILES))) coverage
	lcov -c -d ./coverage -o main_coverage.info
	genhtml main_coverage.info --output-directory coverage_report
	# Cleanup unused cov files
	rm -f *.gcno *.gcda

clean:
	@echo Clean...
	rm -f $(TARGET)
	rm -f *.gcno *.gcda main_coverage.info
	rm -rf ./coverage
	rm -rf ./coverage_report
