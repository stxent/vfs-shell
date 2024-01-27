# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

cmake_minimum_required(VERSION 3.21)
project(Shell CXX)

# Project configuration
option(BUILD_TESTING "Enable testing support." OFF)
option(USE_LTO "Enable Link Time Optimization." OFF)

# Configure build flags
set(FLAGS_PROJECT "-fdata-sections -ffunction-sections -Wall -Wextra -Wshadow")
set(FLAGS_PROJECT_CXX "-fno-exceptions")

if(NOT DEFINED BOARD)
    set(BOARD "Linux")
endif()

if(NOT BUILD_TESTING)
    set(FLAGS_PROJECT_CXX "${FLAGS_PROJECT_CXX} -fno-rtti")
endif()

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

if(USE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_CXX_COMPILE_OPTIONS_IPO -flto=auto -ffat-lto-objects)
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
target_link_options(${PROJECT_NAME} PRIVATE SHELL:${FLAGS_CPU} SHELL:${FLAGS_LINKER})
target_link_libraries(${PROJECT_NAME} PRIVATE project_core project_platform)

if(BUILD_TESTING)
    target_compile_options(project_core PRIVATE --coverage)
    target_compile_options(project_platform PRIVATE --coverage)
    target_link_libraries(${PROJECT_NAME} PRIVATE gcov)

    enable_testing()
    add_subdirectory(Tests)
endif()
