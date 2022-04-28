COMMON_MOCKS=\
	mock/FreeRTOS/mockFreeRTOS.c \
	mock/FreeRTOS/mockQueue.c \
	mock/FreeRTOS/mockTask.c \
	mock/FreeRTOS/mockSemphr.c \
	mock/stm32_hal/MockStm32f7xx_hal.c \
	mock/stm32_hal/MockStm32f7xx_hal_can.c \
	mock/stm32_hal/MockStm32f7xx_hal_adc.c \
	mock/stm32_hal/MockStm32f7xx_hal_uart.c

TEST_SRC=\
	test/comm/can/TestCan.c \
	test/comm/uart/TestUart.c \
	test/io/adc/TestAdc.c

SRC_MOCKS=\
	mock/lib/logging/MockLogging.c