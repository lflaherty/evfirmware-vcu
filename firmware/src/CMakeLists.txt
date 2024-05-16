cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE toolchain.cmake)

project(vcu LANGUAGES C ASM)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_executable(${PROJECT_NAME})
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}.elf")

include_directories(${PROJECT_SOURCE_DIR}/cube-proj/Core/Inc)
include_directories(${PROJECT_SOURCE_DIR}/cube-proj/Lib/FreeRTOS/Source/include)
include_directories(${PROJECT_SOURCE_DIR}/cube-proj/Lib/FreeRTOS/Source/portable/GCC/ARM_CM7/r0p1)
include_directories(${PROJECT_SOURCE_DIR}/cube-proj/Drivers/STM32F7xx_HAL_Driver/Inc)
include_directories(${PROJECT_SOURCE_DIR}/cube-proj/Drivers/CMSIS/Device/ST/STM32F7xx/Include)
include_directories(${PROJECT_SOURCE_DIR}/cube-proj/Drivers/CMSIS/Include)
include_directories(${PROJECT_SOURCE_DIR}/cube-proj/Drivers/STM32F7xx_HAL_Driver/Inc/Legacy)
include_directories(${PROJECT_SOURCE_DIR}/vcu)
include_directories(${PROJECT_SOURCE_DIR}/system-lib)

add_compile_definitions(DEBUG)
add_compile_definitions(USE_HAL_DRIVER)
add_compile_definitions(STM32F767xx)

add_subdirectory(cube-proj)     # main/STM32 HAL init
add_subdirectory(vcu)           # Vehicle and device logic
add_subdirectory(system-lib)    # STM32 library
