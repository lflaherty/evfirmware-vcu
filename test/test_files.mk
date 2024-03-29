# Include system-lib tests:
include systemlib-test/test_files.mk

# Append the different location
COMMON_MOCKS:=$(addprefix systemlib-test/, $(COMMON_MOCKS))
TEST_SRC:=$(addprefix systemlib-test/, $(TEST_SRC))
SRC_MOCKS:=$(addprefix systemlib-test/, $(SRC_MOCKS))

TEST_SRC+=\
	test/vehicleInterface/vehicleState/TestVehicleState.c \
	test/vehicleInterface/vehicleControl/TestVehicleControl.c \
	test/vehicleLogic/stateManager/TestVehicleStateManager.c \
	test/vehicleLogic/stateManager/TestVehicleStateMachine.c \
	test/vehicleLogic/stateManager/TestFaultManager.c \
	test/vehicleLogic/throttleController/TestThrottleController.c \
	test/vehicleLogic/throttleController/TestTorqueMap.c \
	test/vehicleLogic/watchdogTrigger/TestWatchdogTrigger.c \
	test/device/pcdebug/TestPCDebug.c

SRC_MOCKS+=\
	mock/Application/vehicleLogic/stateManager/MockFaultManager.c \
	mock/Application/vehicleLogic/stateManager/MockStateMachine.c \
	mock/Application/vehicleLogic/throttleController/MockThrottleController.c \
	mock/Application/vehicleInterface/vehicleControl/MockVehicleControl.c