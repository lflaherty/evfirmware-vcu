## TestUart
add_executable(TestUart TestUart.c)
# Test harness
target_sources(TestUart PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestUart PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_uart.c)
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_cortex.c)
# Mocks for 1st party
target_sources(TestUart PRIVATE ${PROJECT_SOURCE_DIR}/mock/logging/MockLogging.c)
# Production code
target_sources(TestUart PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/depends/depends.c)


## TestMsgFrameDecode
add_executable(TestMsgFrameDecode TestMsgFrameDecode.c)
# Test harness
target_sources(TestMsgFrameDecode PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestMsgFrameDecode PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
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
target_sources(TestMsgFrameDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/logging/MockLogging.c)
# Production code
target_sources(TestMsgFrameDecode PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/crc/crc.c)
target_sources(TestMsgFrameDecode PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/depends/depends.c)


## TestMsgFrameEncode
add_executable(TestMsgFrameEncode TestMsgFrameEncode.c)
# Test harness
target_sources(TestMsgFrameEncode PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestMsgFrameEncode PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
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
target_sources(TestMsgFrameEncode PRIVATE ${PROJECT_SOURCE_DIR}/mock/logging/MockLogging.c)
# Production code
target_sources(TestMsgFrameEncode PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/crc/crc.c)
target_sources(TestMsgFrameEncode PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/depends/depends.c)


## TestNmeaDecode
add_executable(TestNmeaDecode TestNmeaDecode.c)
# Test harness
target_sources(TestNmeaDecode PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestNmeaDecode PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestNmeaDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestNmeaDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestNmeaDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestNmeaDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestNmeaDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestNmeaDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestNmeaDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
# Mocks for 1st party
target_sources(TestNmeaDecode PRIVATE ${PROJECT_SOURCE_DIR}/mock/logging/MockLogging.c)
# Production code
target_sources(TestNmeaDecode PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/uart/nmeatypes.c)
target_sources(TestNmeaDecode PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/depends/depends.c)


## TestNmeaTypes
add_executable(TestNmeaTypes TestNmeaTypes.c)
# Test harness
target_sources(TestNmeaTypes PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestNmeaTypes PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestNmeaTypes PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestNmeaTypes PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestNmeaTypes PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestNmeaTypes PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestNmeaTypes PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestNmeaTypes PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestNmeaTypes PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
# Mocks for 1st party
target_sources(TestNmeaTypes PRIVATE ${PROJECT_SOURCE_DIR}/mock/logging/MockLogging.c)
# Production code
target_sources(TestNmeaTypes PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/depends/depends.c)
