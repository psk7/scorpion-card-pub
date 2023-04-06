##################################################################################
# "THE ANY BEVERAGE-WARE LICENSE" (Revision 42 - based on beer-ware
# license):
# <dev@layer128.net> wrote this file. As long as you retain this notice
# you can do whatever you want with this stuff. If we meet some day, and
# you think this stuff is worth it, you can buy me a be(ve)er(age) in
# return. (I don't like beer much.)
#
# Matthias Kleemann
##################################################################################


cmake_minimum_required(VERSION 2.8)
### TOOLCHAIN SETUP AREA #################################################
# Set any variables used in the toolchain prior project() call. In that
# case they are already set and used.
##########################################################################

##################################################################################
# tools to be used for programming the AVR
##################################################################################
set(AVR_UPLOADTOOL avrdude)
set(AVR_PROGRAMMER usbasp)
set(AVR_UPLOADTOOL_PORT usb)

#### For Arduino bootloader:
#set(AVR_PROGRAMMER arduino)
#set(AVR_UPLOADTOOL_PORT /dev/tty.wchusbserial1420)
#set(AVR_UPLOADTOOL_BITRATE 57600)


# AVR and fuses needs to be set
set(AVR_MCU atmega328p)
set(AVR_H_FUSE 0xD1)
set(AVR_L_FUSE 0xE7)
set(AVR_EX_FUSE 0×07)
set(MCU_SPEED "16000000UL")

set(CMAKE_CXX_STANDARD 20)

### printf support
#set(EXTRA_FLAGS "-Wl,-u,vfprintf -lprintf_min") 

### END TOOLCHAIN SETUP AREA #############################################
INCLUDE(generic-gcc-avr.cmake)

add_subdirectory(src/USBHost)

##########################################################################
# name your project
##########################################################################
project(sc_multicard)

##################################################################################
# add all sources and headers from ./src
##################################################################################
file(GLOB_RECURSE SRC_C_FILES ${PROJECT_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE SRC_CXX_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/SPI/*.cpp)
file(GLOB_RECURSE SRC_HEADERS ${PROJECT_SOURCE_DIR}/src/*.h)

include_directories(.)
include_directories(src)
include_directories(src/Config)
include_directories(src/SPI)
include_directories(src/USBHost)

add_definitions("-DF_USB=${MCU_SPEED}")
add_definitions("-DUSE_LUFA_CONFIG_HEADER")

##################################################################################
# add AVR executable
##################################################################################
add_avr_executable(
   ${PROJECT_NAME}
   ${SRC_C_FILES} ${SRC_CXX_FILES} ${SRC_HEADERS}
)

target_link_libraries(${PROJECT_NAME}${MCU_TYPE_FOR_FILENAME}.elf usbhost)

find_library(C_LIB c)
message(STATUS "avr-libc: ${C_LIB}")

#avr_target_link_libraries(${PROJECT_NAME} ${C_LIB})


##################################################################################
# add AVR library
##################################################################################
#add_avr_library(
#   <library-name>
#   <sources> <headers>
#)

##################################################################################
# link library to executable
# NOTE: It needs to be the elf target.
##################################################################################
# target_link_libraries(<executable-name>-${AVR_MCU}.elf <library-name>-${AVR_MCU})
# OR easier

