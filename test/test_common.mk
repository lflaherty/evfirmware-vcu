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

# The directory to build in, referenced from location of test_common.mk
# $(TEST_ROOT) references . as being the root of the evfirmware-lib/test directory
# Just have this as $(TEST_ROOT) if building from the evfirmware-lib repo, but change this to
# $(TEST_ROOT)/other_directory if building into a larger set of tests
# BUILD_DIR=$(TEST_ROOT)/../../test
# You can set BUILD_DIR_CD as a make parameter to change this
ifeq ($(BUILD_DIR_CD),)
BUILD_DIR=$(TEST_ROOT)
else
BUILD_DIR=$(TEST_ROOT)/$(BUILD_DIR_CD)
endif

# $(LIB_TEST_DIR) is the relactive directory to locate evfirmware-lib/test from current directory
# just have this as . if building from evfirmare-lib repo, but if building from another set of tests,
# set this to nagivate to evfirmare-lib/test
ifeq ($(LIB_TEST_DIR),)
LIB_TEST_DIR=.
endif

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
	$(MOCKS) \
	$(TESTED_SRC) \
	test/$(TEST_SRC) \
	)

INC_DIRS=-I$(LIB_TEST_DIR)/../src
INC_DIRS+=-I$(LIB_TEST_DIR)/test
INC_DIRS+=-I$(LIB_TEST_DIR)/mock
INC_DIRS+=-I$(LIB_TEST_DIR)/$(UNITY_ROOT)/src
INC_DIRS+=-I$(LIB_TEST_DIR)/$(UNITY_ROOT)/extras/fixture/src
SYMBOLS=-DUNITY_FIXTURE_NO_EXTRAS

default:
	@echo Build...
	mkdir -p $(BUILD_DIR)/coverage
	cd $(BUILD_DIR) && pwd && $(GCC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) $(LFLAGS) -o $(TARGET)
	mv $(patsubst %.c,$(BUILD_DIR)/%.gcno,$(notdir $(TESTED_SRC))) $(BUILD_DIR)/coverage
