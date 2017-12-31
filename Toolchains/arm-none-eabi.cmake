#Copyright (C) 2017 xent
#Project is distributed under the terms of the GNU General Public License v3.0

cmake_minimum_required(VERSION 3.6)

set(CROSS_COMPILE "arm-none-eabi-" CACHE STRING "Cross compiler prefix")

set(CMAKE_C_COMPILER "${CROSS_COMPILE}gcc")
set(CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++")

set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
