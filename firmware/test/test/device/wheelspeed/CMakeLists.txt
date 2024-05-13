## TestWheelspeed
add_executable(TestWheelspeed TestWheelspeed.c)
# Test harness
target_sources(TestWheelspeed PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestWheelspeed PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_gpio.c)
# Mocks for 1st party
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/time/tasktimer/MockTasktimer.c)
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/mock/lib/logging/MockLogging.c)
# Production code
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-src/io/gpio/gpio.c)
target_sources(TestWheelspeed PRIVATE ${PROJECT_SOURCE_DIR}/systemlib-src/lib/depends/depends.c)
target_sources(TestWheelspeed PRIVATE ${FIRMWARE_SRC_DIR}/vcu/vehicleInterface/vehicleState/vehicleState.c)