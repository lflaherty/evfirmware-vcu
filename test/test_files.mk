COMMON_MOCKS=\
	mock/FreeRTOS/mockFreeRTOS.c \
	mock/FreeRTOS/mockQueue.c \
	mock/FreeRTOS/mockStreamBuffer.c \
	mock/FreeRTOS/mockTask.c \
	mock/FreeRTOS/mockSemphr.c \
	mock/stm32_hal/MockStm32f7xx_hal.c \
	mock/stm32_hal/MockStm32f7xx_hal_can.c \
	mock/stm32_hal/MockStm32f7xx_hal_adc.c \
	mock/stm32_hal/MockStm32f7xx_hal_gpio.c \
	mock/stm32_hal/MockStm32f7xx_hal_uart.c \
	mock/stm32_hal/MockStm32f7xx_hal_tim.c \
	mock/stm32_hal/MockStm32f7xx_hal_crc.c

TEST_SRC=\
	test/comm/can/TestCan.c \
	test/comm/uart/TestUart.c \
	test/comm/uart/TestMsgFrameDecode.c \
	test/comm/uart/TestMsgFrameEncode.c \
	test/io/adc/TestAdc.c \
	test/io/gpio/TestGpio.c \
	test/time/tasktimer/TestTaskTimer.c \
	test/lib/logging/TestLogging.c

SRC_MOCKS=\
	mock/lib/logging/MockLogging.c \
	mock/time/tasktimer/MockTaskTimer.c