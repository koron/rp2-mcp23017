cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(mcp23017 C CXX ASM)

# Initialise the Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(mcp23017
	main.c
	onboard_led.c
	)

# Link Time Optimization is not supported yet.
#set_property(TARGET mcp23017 PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

pico_set_program_name(mcp23017 "mcp23017")
pico_set_program_version(mcp23017 "0.1")

pico_enable_stdio_uart(mcp23017 1)
pico_enable_stdio_usb(mcp23017 0)

target_include_directories(mcp23017 PRIVATE
	${CMAKE_CURRENT_LIST_DIR}
	)

target_link_libraries(mcp23017
	pico_stdlib
	hardware_i2c
	# for debug
	pico_bootsel_via_double_reset
	)

pico_add_extra_outputs(mcp23017)
