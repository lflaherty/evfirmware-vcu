## TestThrottleController
add_executable(TestThrottleController TestThrottleController.c)
# Test harness
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/Unity/src/unity.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockQueue.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockTask.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockSemphr.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/std/MockStdio.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/stm32_hal/MockStm32f7xx_hal_can.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/stm32_hal/MockStm32f7xx_hal_crc.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/stm32_hal/MockStm32f7xx_hal_uart.c)
# Mocks for 1st party
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/lib/logging/MockLogging.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/time/tasktimer/MockTasktimer.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/vehicleInterface/vehicleControl/MockVehicleControl.c)
# Production code
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-src/comm/can/can.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-src/lib/crc/crc.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-src/lib/depends/depends.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/../src/vcu/vehicleInterface/vehicleState/vehicleState.c)
target_sources(TestThrottleController PRIVATE ${PROJECT_SOURCE_DIR}/../src/vcu/vehicleLogic/throttleController/torqueMap.c)


## TestTorqueMap
add_executable(TestTorqueMap TestTorqueMap.c)
# Test harness
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/Unity/src/unity.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockQueue.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockTask.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/FreeRTOS/mockSemphr.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/std/MockStdio.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/stm32_hal/MockStm32f7xx_hal_can.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/stm32_hal/MockStm32f7xx_hal_crc.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/stm32_hal/MockStm32f7xx_hal_uart.c)
# Mocks for 1st party
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/lib/logging/MockLogging.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-test/mock/time/tasktimer/MockTasktimer.c)
# Production code
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-src/comm/can/can.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-src/lib/crc/crc.c)
target_sources(TestTorqueMap PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-src/lib/depends/depends.c)
