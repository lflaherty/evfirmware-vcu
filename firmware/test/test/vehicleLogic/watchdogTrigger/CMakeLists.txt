## TestWatchdogTrigger
add_executable(TestWatchdogTrigger TestWatchdogTrigger.c)
# Test harness
target_sources(TestWatchdogTrigger PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestWatchdogTrigger PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_can.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_crc.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_gpio.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_uart.c)
# Mocks for 1st party
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/lib/logging/MockLogging.c)
target_sources(TestWatchdogTrigger PRIVATE ${PROJECT_SOURCE_DIR}/mock/time/tasktimer/MockTasktimer.c)
# Production code
target_sources(TestWatchdogTrigger PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/comm/can/can.c)
target_sources(TestWatchdogTrigger PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/io/gpio/gpio.c)
target_sources(TestWatchdogTrigger PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/lib/crc/crc.c)
target_sources(TestWatchdogTrigger PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/lib/depends/depends.c)