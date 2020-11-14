# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

if(NOT DEFINED BOARD)
    message(FATAL_ERROR "BOARD not defined")
endif()

include_directories(
        "${PROJECT_SOURCE_DIR}/Platform/${BOARD}"
)

file(GLOB_RECURSE BOARD_SOURCES
        "${PROJECT_SOURCE_DIR}/Platform/${BOARD}/*.c"
        "${PROJECT_SOURCE_DIR}/Platform/${BOARD}/*.cpp"
)

include("${PROJECT_SOURCE_DIR}/Platform/${BOARD}/CMakeLists.txt" OPTIONAL)
