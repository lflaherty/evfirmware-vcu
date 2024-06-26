## TestVehicleControl
add_executable(TestVehicleControl TestVehicleControl.c)
# Test harness
target_sources(TestVehicleControl PRIVATE ${THIRD_PARTY_DIR}/Unity/src/unity.c)
target_sources(TestVehicleControl PRIVATE ${THIRD_PARTY_DIR}/Unity/extras/fixture/src/unity_fixture.c)
# Mocks for 3rd party
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockFreeRTOS.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockQueue.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockStreamBuffer.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockTask.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/FreeRTOS/mockSemphr.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/std/MockStdio.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_can.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_crc.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_gpio.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/stm32_hal/MockStm32f7xx_hal_uart.c)
# Mocks for 1st party
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/logging/MockLogging.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/tasktimer/MockTasktimer.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/device/inverter/MockCInverter.c)
target_sources(TestVehicleControl PRIVATE ${PROJECT_SOURCE_DIR}/mock/Application/device/pdm/MockPdm.c)
# Production code
target_sources(TestVehicleControl PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/can/can.c)
target_sources(TestVehicleControl PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/gpio/gpio.c)
target_sources(TestVehicleControl PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/depends/depends.c)
target_sources(TestVehicleControl PRIVATE ${FIRMWARE_SRC_DIR}/system-lib/crc/crc.c)
target_sources(TestVehicleControl PRIVATE ${FIRMWARE_SRC_DIR}/vcu/device/sdc/sdc.c)
target_sources(TestVehicleControl PRIVATE ${FIRMWARE_SRC_DIR}/vcu/device/dashboard_output/dashboard_output.c)
target_sources(TestVehicleControl PRIVATE ${FIRMWARE_SRC_DIR}/vcu/vehicleInterface/vehicleState/vehicleState.c)
