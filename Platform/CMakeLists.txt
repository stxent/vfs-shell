# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

if(NOT DEFINED PLATFORM)
    message(FATAL_ERROR "PLATFORM not defined")
endif()

include_directories(
        "${PROJECT_SOURCE_DIR}/Platform/${PLATFORM}"
)

file(GLOB_RECURSE PLATFORM_SOURCES
        "${PROJECT_SOURCE_DIR}/Platform/${PLATFORM}/*.c"
        "${PROJECT_SOURCE_DIR}/Platform/${PLATFORM}/*.cpp"
)

include("${PROJECT_SOURCE_DIR}/Platform/${PLATFORM}/CMakeLists.txt" OPTIONAL)
