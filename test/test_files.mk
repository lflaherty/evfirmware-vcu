# Include system-lib tests:
include systemlib-test/test_files.mk

# Append the different location
COMMON_MOCKS:=$(addprefix systemlib-test/, $(COMMON_MOCKS))
TEST_SRC:=$(addprefix systemlib-test/, $(TEST_SRC))
SRC_MOCKS:=$(addprefix systemlib-test/, $(SRC_MOCKS))

TEST_SRC+=\
	test/vehicleInterface/vehicleState/TestVehicleState.c