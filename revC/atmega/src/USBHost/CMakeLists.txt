set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 3.22)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
project(usbhost C CXX ASM)

##########################################################################
# executables in use
##########################################################################
find_program(AVR_CC avr-gcc)
find_program(AVR_CXX avr-g++)
find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_SIZE_TOOL avr-size)
find_program(AVR_OBJDUMP avr-objdump)

set(CMAKE_SYSTEM_NAME generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})

include_directories(.. ../../src/SPI ../src/SPI ../src)

add_definitions("-mmcu=${AVR_MCU}")
add_definitions("-Wno-volatile")
add_definitions("-Wno-unknown-pragmas")
add_definitions("-Wno-class-memaccess")
add_definitions("-Wno-array-bounds")
add_definitions("-Wno-packed")
add_definitions("-Wno-unused-but-set-variable")

add_compile_options(-Og -g -gdwarf-4)
#add_compile_options(-Os -flto)

file(GLOB_RECURSE SOURCES "*.*")

add_library(usbhost STATIC ${SOURCES})
