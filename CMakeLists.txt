# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

project(Shell CXX)
cmake_minimum_required(VERSION 3.6)
cmake_policy(SET CMP0054 NEW)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "The type of build: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

option(USE_LTO "Enable Link Time Optimization." OFF)

# Configure build flags
set(FLAGS_DEFAULT "-fdata-sections -ffunction-sections -Wall -Wextra -Wshadow -pedantic -fno-rtti -fno-exceptions")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_DEFAULT}")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

if(${USE_LTO})
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto -ffat-lto-objects")
endif()

include_directories(SYSTEM
        "Libs/xcore/include"
        "Libs/halm/include"
        "Libs/osw/include"
        "Libs/yaf/include"
)

include("Core/CMakeLists.txt")
include("Platform/CMakeLists.txt")

# Setup symbols for library headers
string(TOLOWER ${CMAKE_SYSTEM_NAME} OS_TYPE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DOS_TYPE=${OS_TYPE}")

if(DEFINED CMAKE_SYSTEM_SOC)
    string(TOUPPER "${CMAKE_SYSTEM_SOC}" HALM_PLATFORM)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -D${HALM_PLATFORM}")
endif()

# Setup libraries, script should be included after Core and Platform scripts
include("Libs/CMakeLists.txt")

# Configure executable
add_executable(${PROJECT_NAME} ${CORE_SOURCES} ${PLATFORM_SOURCES})
add_dependencies(${PROJECT_NAME} yaf osw halm xcore)
target_link_libraries(${PROJECT_NAME} ${PLATFORM_LIBS})
