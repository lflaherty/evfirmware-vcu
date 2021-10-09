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

UNITY_ROOT=./Unity

TARGET=all_tests
SRC_FILES=\
	  $(UNITY_ROOT)/src/unity.c \
	  $(UNITY_ROOT)/extras/fixture/src/unity_fixture.c \
	  mock/MockLogging.c \
	  test/lib/logging/TestLogMock.c \
	  test/comm/can/TestCan.c \
	  test/comm/can/TestCanRunner.c \
	  test/all_tests.c

INC_DIRS=-I../src -Itest -Imock -I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src
SYMBOLS=-DUNITY_FIXTURE_NO_EXTRAS

all: clean default

default:
	$(GCC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES) -o $(TARGET)

clean:
	rm -f $(TARGET)
