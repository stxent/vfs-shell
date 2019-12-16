# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

include_directories(
        "${PROJECT_SOURCE_DIR}/Core"
)

file(GLOB_RECURSE CORE_SOURCES
        "${PROJECT_SOURCE_DIR}/Core/*.c"
        "${PROJECT_SOURCE_DIR}/Core/*.cpp"
)
