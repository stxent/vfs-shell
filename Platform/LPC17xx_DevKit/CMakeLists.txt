# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Xlinker -T ${PROJECT_SOURCE_DIR}/Platform/${PLATFORM}/Memory.ld")

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_PROCESSOR "cortex-m3")
set(CMAKE_SYSTEM_SOC "lpc17xx")

set(FLAGS_SUBPROJECT_YAF "-DCONFIG_FLAG_THREADS=OFF -DCONFIG_NAME_LENGTH=256")

include("${PROJECT_SOURCE_DIR}/Platform/Targets/cortex-m.cmake")
