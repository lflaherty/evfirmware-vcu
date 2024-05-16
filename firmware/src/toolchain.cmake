# Adapted from https://github.com/jobroe/cmake-arm-embedded

# Append current directory to CMAKE_MODULE_PATH for making device specific cmake modules visible
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Target definition
set(CMAKE_SYSTEM_NAME  Generic)
set(CMAKE_SYSTEM_PROCESSOR ARM)

#---------------------------------------------------------------------------------------
# Set toolchain paths
#---------------------------------------------------------------------------------------
set(TOOLCHAIN arm-none-eabi)

# Set system depended extensions
if(WIN32)
    set(TOOLCHAIN_EXT ".exe" )
else()
    set(TOOLCHAIN_EXT "" )
endif()

# Perform compiler test with static library
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)


#---------------------------------------------------------------------------------------
# Set compilers
#---------------------------------------------------------------------------------------
set(CMAKE_C_COMPILER ${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "C Compiler")
set(CMAKE_ASM_COMPILER ${TOOLCHAIN}-gcc${TOOLCHAIN_EXT} CACHE INTERNAL "ASM Compiler")

set(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_PREFIX}/${${TOOLCHAIN}} ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)


#---------------------------------------------------------------------------------------
# Set compiler/linker flags
#---------------------------------------------------------------------------------------

# Object build options for STM32F767
set(OBJECT_GEN_FLAGS "")
set(OBJECT_GEN_FLAGS "${OBJECT_GEN_FLAGS} -O0")
set(OBJECT_GEN_FLAGS "${OBJECT_GEN_FLAGS} -g3")
set(OBJECT_GEN_FLAGS "${OBJECT_GEN_FLAGS} -Wall")
set(OBJECT_GEN_FLAGS "${OBJECT_GEN_FLAGS} -fno-builtin -ffunction-sections -fdata-sections -fstack-usage -fomit-frame-pointer")
set(OBJECT_GEN_FLAGS "${OBJECT_GEN_FLAGS} -mcpu=cortex-m7 -mthumb -mfpu=fpv5-d16 -mfloat-abi=hard")

set(CMAKE_C_FLAGS   "${OBJECT_GEN_FLAGS} -std=gnu11 " CACHE INTERNAL "C Compiler options")
set(CMAKE_ASM_FLAGS "${OBJECT_GEN_FLAGS} -x assembler-with-cpp " CACHE INTERNAL "ASM Compiler options")

set(CMAKE_EXE_LINKER_FLAGS "" CACHE INTERNAL "Linker options")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--gc-sections" CACHE INTERNAL "Linker options")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map" CACHE INTERNAL "Linker options")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static --specs=nosys.specs" CACHE INTERNAL "Linker options")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mfpu=fpv5-d16 -mfloat-abi=hard -mthumb" CACHE INTERNAL "Linker options")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--start-group -lc -lm -Wl,--end-group" CACHE INTERNAL "Linker options")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${PROJECT_SOURCE_DIR}/cube-proj/STM32F767ZITX_FLASH.ld")
