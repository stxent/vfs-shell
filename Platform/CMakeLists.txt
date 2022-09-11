# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

include("${PROJECT_SOURCE_DIR}/Platform/${BOARD}/CMakeLists.txt" OPTIONAL)

file(GLOB_RECURSE BOARD_SOURCES
        "${PROJECT_SOURCE_DIR}/Platform/${BOARD}/*.c"
        "${PROJECT_SOURCE_DIR}/Platform/${BOARD}/*.cpp"
)

add_library(project_platform OBJECT ${BOARD_SOURCES})
target_include_directories(project_platform PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}")
target_link_libraries(project_platform PRIVATE project_core halm dpm)
