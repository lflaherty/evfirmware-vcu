# Append this to the bottom of a makefile 

# Check for required vars
ifneq ($(and $(TEST_ROOT),$(TARGET),$(MOCKS),$(TESTED_SRC),$(TEST_SRC)),)
all: default
else
all:
$(info required vars not set!)
endif

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

BUILD_DIR=$(TEST_ROOT)
LIB_TEST_DIR=../System/test

UNITY_ROOT=Unity

COMMON_MOCKS=\
	mock/FreeRTOS/mockFreeRTOS.c \
	mock/FreeRTOS/mockQueue.c \
	mock/FreeRTOS/mockTask.c \
	mock/FreeRTOS/mockSemphr.c \
	mock/stm32_hal/MockStm32f7xx_hal.c \
	mock/stm32_hal/MockStm32f7xx_hal_can.c \
	mock/stm32_hal/MockStm32f7xx_hal_adc.c

SRC_FILES=$(addprefix $(LIB_TEST_DIR)/,\
	$(UNITY_ROOT)/src/unity.c \
	$(UNITY_ROOT)/extras/fixture/src/unity_fixture.c \
	$(COMMON_MOCKS) \
	$(MOCKS)) \
	$(TESTED_SRC) \
	test/$(TEST_SRC)

INC_DIRS=-I../Application
INC_DIRS+=-I$(LIB_TEST_DIR)/../src
INC_DIRS+=-I$(LIB_TEST_DIR)/test
INC_DIRS+=-I$(LIB_TEST_DIR)/mock
INC_DIRS+=-I$(LIB_TEST_DIR)/$(UNITY_ROOT)/src
INC_DIRS+=-I$(LIB_TEST_DIR)/$(UNITY_ROOT)/extras/fixture/src
SYMBOLS=-DUNITY_FIXTURE_NO_EXTRAS

default:
	@echo Build...
	mkdir -p $(BUILD_DIR)/coverage
	cd $(BUILD_DIR) && $(GCC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) $(LFLAGS) -o $(TARGET)
	mv $(patsubst %.c,$(BUILD_DIR)/%.gcno,$(notdir $(TESTED_SRC))) $(BUILD_DIR)/coverage
