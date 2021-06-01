cmake_minimum_required(VERSION 3.12)
set(MCU "" CACHE STRING "Target MCU.")
string(TOLOWER ${MCU} MCU)
if(NOT MCU MATCHES "^(atmega16u4|atmega32u4|atmega32u6)$")
	message(FATAL_ERROR "Unrecognized MCU '${MCU}'")
endif()
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_C_FLAGS "-mmcu=${MCU} -Os -g")
find_program(AVROBJCOPY avr-objcopy)
find_program(AVRDUDE avrdude)
find_program(TEENSYLOADER NAMES teensy_loader_cli teensy-loader-cli)
project(rcmixer C)
add_definitions(-Wall -Wextra -Wpedantic -Wundef -Wshadow -Wredundant-decls -Wstrict-prototypes -Wmissing-prototypes
	-Wno-variadic-macros -Wno-unused-result -Wno-unused-parameter -Wno-unused-function)
add_executable(main.elf src/main.c)
target_compile_options(main.elf PRIVATE -Wa,-adhlns=main.lst)
add_custom_command(
	OUTPUT main.ihx
	COMMAND ${AVROBJCOPY} -j .text -j .data -O ihex main.elf main.ihx
	DEPENDS main.elf
)
add_custom_target(main ALL DEPENDS main.ihx)
add_custom_target(flash COMMAND ${CMAKE_BINARY_DIR}/flash)
add_custom_target(flash_teensy COMMAND ${TEENSYLOADER} -w -v --mcu=${MCU} main.ihx)
configure_file(etc/flash.in flash @ONLY)
