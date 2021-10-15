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

UNITY_ROOT=Unity

COMMON_MOCKS=\
	mock/FreeRTOS/mockFreeRTOS.c \
	mock/FreeRTOS/mockQueue.c \
	mock/FreeRTOS/mockTask.c \
	mock/stm32_hal/MockStm32f7xx_hal.c \
	mock/stm32_hal/MockStm32f7xx_hal_can.c \
	mock/stm32_hal/MockStm32f7xx_hal_adc.c \

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
	cd $(TEST_ROOT) && $(GCC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) $(LFLAGS) -o $(TARGET)
	mv $(patsubst %.c,$(TEST_ROOT)/%.gcno,$(notdir $(TESTED_SRC))) $(TEST_ROOT)/coverage
