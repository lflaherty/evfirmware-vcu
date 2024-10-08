cmake_minimum_required (VERSION 2.8.11)
project(ev_test)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(FIRMWARE_SRC_DIR ${PROJECT_SOURCE_DIR}/../src)
set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/../third_party)

include_directories(${FIRMWARE_SRC_DIR}/vcu)
include_directories(${FIRMWARE_SRC_DIR}/system-lib)
include_directories(${PROJECT_SOURCE_DIR}/test)
include_directories(${PROJECT_SOURCE_DIR}/mock)
include_directories(${PROJECT_SOURCE_DIR}/mock/Application)
include_directories(${THIRD_PARTY_DIR}/Unity/src)
include_directories(${THIRD_PARTY_DIR}/Unity/extras/fixture/src)
include_directories(${THIRD_PARTY_DIR}/Unity/extras/memory/src)

set_property(GLOBAL PROPERTY C_STANDARD 11)

# Compile flags
# Warning flags
add_compile_options(-Wall)
add_compile_options(-Wpointer-arith)
add_compile_options(-Wcast-align)
add_compile_options(-Wwrite-strings)
add_compile_options(-Wswitch-default)
add_compile_options(-Wunreachable-code)
add_compile_options(-Winit-self)
add_compile_options(-Wmissing-field-initializers)
add_compile_options(-Wno-unknown-pragmas)
add_compile_options(-Wstrict-prototypes)
add_compile_options(-Wundef)
add_compile_options(-Wold-style-definition)
add_compile_options(-Wdouble-promotion)
add_compile_options(-Wshadow)
add_compile_options(-Wformat=2)
add_compile_options(-Wconversion)
add_compile_options(-Werror)
# Profiling flags
add_compile_options(-g)
add_compile_options(--coverage)
# Sanitizers
add_compile_options(-fsanitize=address)
add_compile_options(-fsanitize=pointer-compare)
add_compile_options(-fsanitize=pointer-subtract)
add_compile_options(-fsanitize=undefined)

add_link_options(-lm)
add_link_options(-fsanitize=address)
add_link_options(-fsanitize=pointer-compare)
add_link_options(-fsanitize=pointer-subtract)
add_link_options(-fsanitize=undefined)
add_link_options(-g)
add_link_options(--coverage)

add_subdirectory(test)
