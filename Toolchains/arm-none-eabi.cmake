#Copyright (C) 2017 xent
#Project is distributed under the terms of the GNU General Public License v3.0

cmake_minimum_required(VERSION 3.6)

set(CMAKE_SYSTEM_NAME "Generic")

set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
set(CMAKE_SIZE "arm-none-eabi-size")

# Disable linking stage because cross-compiling toolchain cannot link without custom linker script
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
