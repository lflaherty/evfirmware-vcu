## TestFaultManager
add_executable(TestFaultManager TestFaultManager.c)
# Test harness
target_sources(TestFaultManager PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestFaultManager PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_can.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_crc.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_tim.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_uart.c)
# Mocks for 1st party
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/lib/logging/MockLogging.c)
target_sources(TestFaultManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/time/tasktimer/MockTasktimer.c)
# Production code
target_sources(TestFaultManager PRIVATE ${FIRMWARE_SRC_DIR}/vcu/vehicleInterface/vehicleState/vehicleState.c)
target_sources(TestFaultManager PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/comm/can/can.c)
target_sources(TestFaultManager PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/lib/depends/depends.c)
target_sources(TestFaultManager PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/lib/crc/crc.c)


## TestVehicleStateMachine
add_executable(TestVehicleStateMachine TestVehicleStateMachine.c)
# Test harness
target_sources(TestVehicleStateMachine PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestVehicleStateMachine PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_can.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_crc.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_uart.c)
# Mocks for 1st party
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/lib/logging/MockLogging.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/time/tasktimer/MockTasktimer.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/vehicleInterface/vehicleControl/MockVehicleControl.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/vehicleLogic/stateManager/MockFaultManager.c)
target_sources(TestVehicleStateMachine PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/vehicleLogic/throttleController/MockThrottleController.c)
# Production code
target_sources(TestVehicleStateMachine PRIVATE ${FIRMWARE_SRC_DIR}/vcu/vehicleInterface/vehicleState/vehicleState.c)
target_sources(TestVehicleStateMachine PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/lib/depends/depends.c)


## TestVehicleStateManager
add_executable(TestVehicleStateManager TestVehicleStateManager.c)
# Test harness
target_sources(TestVehicleStateManager PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestVehicleStateManager PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_can.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_crc.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_uart.c)
# Mocks for 1st party
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/lib/logging/MockLogging.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/time/tasktimer/MockTasktimer.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/vehicleInterface/vehicleControl/MockVehicleControl.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/vehicleLogic/stateManager/MockFaultManager.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/vehicleLogic/stateManager/MockStateMachine.c)
target_sources(TestVehicleStateManager PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/vehicleLogic/throttleController/MockThrottleController.c)
# Production code
target_sources(TestVehicleStateManager PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/lib/depends/depends.c)
