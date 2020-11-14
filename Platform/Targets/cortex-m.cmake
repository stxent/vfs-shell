# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

set(FLAGS_CPU "-mcpu=${CMAKE_SYSTEM_PROCESSOR} -mthumb -nostartfiles")

if("${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "cortex-m4-fpu")
    set(FLAGS_CPU "${FLAGS_CPU} -mfloat-abi=hard -mfpu=fpv4-sp-d16")
endif()

string(TOUPPER ${CMAKE_SYSTEM_PLATFORM} PLATFORM)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_CPU} ${FLAGS_PLATFORM}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker --gc-sections --specs=nosys.specs --specs=nano.specs")
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
