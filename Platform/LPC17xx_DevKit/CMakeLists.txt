# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

set(FLAGS_LINKER "--specs=nosys.specs --specs=nano.specs -Wl,--gc-sections -T ${PROJECT_SOURCE_DIR}/Platform/${BOARD}/Memory.ld" PARENT_SCOPE)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")
