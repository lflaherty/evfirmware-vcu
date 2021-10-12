# Append this to the bottom of a makefile 

# Check for required vars
ifneq ($(and $(TEST_ROOT),$(TARGET),$(MOCKS),$(TESTED_SRC),$(TEST_SRC)),)
all: default
else
all:
$(info required vars not set!)
endif

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
CFLAGS += -g -fprofile-arcs -ftest-coverage -fprofile-dir=./coverage

UNITY_ROOT=Unity

COMMON_MOCKS=\
	mock/FreeRTOS/mockFreeRTOS.c \
	mock/FreeRTOS/mockQueue.c \
	mock/FreeRTOS/mockTask.c \
	mock/stm32_hal/MockStm32f7xx_hal.c \
	mock/stm32_hal/MockStm32f7xx_hal_can.c \

SRC_FILES=\
	$(UNITY_ROOT)/src/unity.c \
	$(UNITY_ROOT)/extras/fixture/src/unity_fixture.c \
	$(COMMON_MOCKS) \
	$(MOCKS) \
	$(TESTED_SRC) \
	test/$(TEST_SRC)

INC_DIRS=-I../src -Itest -Imock -I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src
SYMBOLS=-DUNITY_FIXTURE_NO_EXTRAS

default:
	@echo Build...
	mkdir -p $(TEST_ROOT)/coverage
	cd $(TEST_ROOT) && $(GCC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) -o $(TARGET)
	mv $(TEST_ROOT)/can.gcno $(TEST_ROOT)/coverage
