cmake_minimum_required(VERSION 3.15)
set(MCU "" CACHE STRING "Target MCU.")
string(TOLOWER ${MCU} MCU)
if(NOT MCU MATCHES "^(atmega16u4|atmega32u4|atmega32u6)$")
	message(FATAL_ERROR "Unrecognized MCU '${MCU}'")
endif()
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_C_COMPILER avr-gcc)
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_C_FLAGS "-mmcu=${MCU} -Os")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g")
find_program(TEENSYLOADER NAMES teensy_loader_cli teensy-loader-cli)
project(RC-Dual-Motor-Driver C)
add_compile_options(-ffreestanding -ffunction-sections -fdata-sections -fsingle-precision-constant
	-Wall -Wextra -Wpedantic -Wundef -Wshadow -Wredundant-decls -Wstrict-prototypes -Wmissing-prototypes
	-Wno-variadic-macros -Wno-unused-result -Wno-unused-parameter)
add_executable(main.elf src/main.c)
target_compile_options(main.elf PRIVATE -Wa,-adhlns=main.lst)
add_custom_command(
	OUTPUT main.hex
	COMMAND avr-objcopy -O ihex main.elf main.hex
	DEPENDS main.elf
)
add_custom_target(main ALL DEPENDS main.hex)
add_custom_target(flash COMMAND ${CMAKE_BINARY_DIR}/flash DEPENDS main.hex)
add_custom_target(flash-teensy COMMAND ${TEENSYLOADER} -w -v --mcu=${MCU} main.hex DEPENDS main.hex)
configure_file(etc/flash.in flash @ONLY)
