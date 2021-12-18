cmake_minimum_required(VERSION 3.15)
project(RC-Dual-Motor-Driver C)
set(MCU "" CACHE STRING "Target MCU.")
string(TOLOWER ${MCU} MCU)
if(NOT MCU MATCHES "^(atmega16u4|atmega32u4|atmega32u6)$")
	message(FATAL_ERROR "Unrecognized MCU '${MCU}'")
endif()
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_C_FLAGS "-mmcu=${MCU} -Os")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
set(OBJCOPY avr-objcopy)
find_program(AVRDUDE avrdude)
find_program(TEENSYLOADER NAMES teensy_loader_cli teensy-loader-cli)
add_compile_options(-ffreestanding -ffunction-sections -fdata-sections -fsingle-precision-constant
	-Wall -Wextra -Wpedantic -Wundef -Wshadow -Wredundant-decls -Wstrict-prototypes -Wmissing-prototypes
	-Wno-variadic-macros -Wno-unused-result -Wno-unused-parameter)
add_executable(main.elf src/main.c)
target_compile_options(main.elf PRIVATE -Wa,-adhlns=main.lst)
add_custom_command(
	OUTPUT main.ihx
	COMMAND ${OBJCOPY} -O ihex main.elf main.ihx
	DEPENDS main.elf
)
add_custom_target(main ALL DEPENDS main.ihx)
add_custom_target(flash COMMAND ${CMAKE_BINARY_DIR}/flash DEPENDS main.ihx)
add_custom_target(flash-teensy COMMAND ${TEENSYLOADER} -w -v --mcu=${MCU} main.ihx DEPENDS main.ihx)
configure_file(etc/flash.in flash @ONLY)
