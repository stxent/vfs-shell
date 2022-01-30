# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

include_directories(
        "${PROJECT_SOURCE_DIR}/Core"
)

file(GLOB_RECURSE CORE_SOURCES
        "${PROJECT_SOURCE_DIR}/Core/*.c"
        "${PROJECT_SOURCE_DIR}/Core/*.cpp"
)

# Core package
add_library(project_core OBJECT ${CORE_SOURCES})
target_include_directories(project_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(project_core PUBLIC osw yaf)
