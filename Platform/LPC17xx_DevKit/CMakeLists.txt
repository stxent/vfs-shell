# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker -T ${PROJECT_SOURCE_DIR}/Platform/${BOARD}/Memory.ld")
set(CMAKE_SYSTEM_PROCESSOR "cortex-m3")
set(CMAKE_SYSTEM_PLATFORM "lpc17xx")

include("${PROJECT_SOURCE_DIR}/Platform/Targets/cortex-m.cmake")
