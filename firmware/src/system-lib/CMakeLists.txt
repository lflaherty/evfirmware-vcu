# Comm
add_subdirectory(can)
add_subdirectory(i2c)
# add_subdirectory(spi)  # SPI only used in expansion board, not currently used
add_subdirectory(uart)

# IO
add_subdirectory(adc)
add_subdirectory(gpio)

# Time
add_subdirectory(rtc)
add_subdirectory(tasktimer)

# Lib
add_subdirectory(crc)
add_subdirectory(depends)
add_subdirectory(logging)
