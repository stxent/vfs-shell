# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

project(Shell CXX)
cmake_minimum_required(VERSION 3.13)
cmake_policy(SET CMP0054 NEW)
cmake_policy(SET CMP0069 NEW)

option(BUILD_TESTING "Enable testing support." OFF)
option(USE_LTO "Enable Link Time Optimization." OFF)

# Configure build flags
set(FLAGS_DEFAULT "-fdata-sections -ffunction-sections -Wall -Wextra -Wshadow -pedantic -fno-exceptions")

if(NOT DEFINED BOARD)
    set(BOARD "Linux")
endif()

if(NOT ${BUILD_TESTING})
    set(FLAGS_DEFAULT "${FLAGS_DEFAULT} -fno-rtti")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_DEFAULT} ${FLAGS_CPU}")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

if(${USE_LTO})
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_COMPILE_OPTIONS_IPO -flto -ffat-lto-objects)
    endif()
endif()

# Configure XCORE library
set(FS_NAME_LENGTH 256 CACHE INTERNAL "Filesystem name length" FORCE)
add_subdirectory(Libs/xcore xcore)

# Configure OSW library
add_subdirectory(Libs/osw osw)

# Configure HALM library, CONFIG_FILE should be defined
set(HALM_CONFIG_FILE "${PROJECT_SOURCE_DIR}/Platform/${BOARD}/Halm.config" CACHE INTERNAL "Path to the library configuration file" FORCE)
add_subdirectory(Libs/halm halm)

# Configure YAF library
set(YAF_CONFIG_THREADS OFF CACHE INTERNAL "Disable threads" FORCE)
add_subdirectory(Libs/yaf yaf)

# Configure DPM library
add_subdirectory(Libs/dpm dpm)

# Add Core and Platform libraries
add_subdirectory(Core)
add_subdirectory(Platform)

# Configure executable
add_executable(${PROJECT_NAME} $<TARGET_OBJECTS:project_core> $<TARGET_OBJECTS:project_platform>)
target_link_libraries(${PROJECT_NAME} PRIVATE project_core project_platform)

if(${BUILD_TESTING})
    target_compile_options(project_core PRIVATE --coverage)
    target_compile_options(project_platform PRIVATE --coverage)
    target_link_libraries(${PROJECT_NAME} PRIVATE gcov)

    enable_testing()
    add_subdirectory(Tests)
endif()
