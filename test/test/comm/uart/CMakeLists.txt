## TestUart
add_executable(TestUart TestUart.c)
# Test harness
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/Unity/src/unity.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_uart.c)
# Mocks for 1st party
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/lib/logging/MockLogging.c)


## TestMsgFrameDecode
add_executable(TestMsgFrameDecode TestMsgFrameDecode.c)
# Test harness
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/Unity/src/unity.c)
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_crc.c)
# Mocks for 1st party
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/lib/logging/MockLogging.c)
# Production code
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/../src/lib/crc/crc.c)


## TestMsgFrameEncode
add_executable(TestMsgFrameEncode TestMsgFrameEncode.c)
# Test harness
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/Unity/src/unity.c)
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_crc.c)
# Mocks for 1st party
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/lib/logging/MockLogging.c)
# Production code
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/../src/lib/crc/crc.c)