# Copyright (C) 2017 xent
# Project is distributed under the terms of the GNU General Public License v3.0

include(ExternalProject)

set(FLAGS_SUBPROJECT "")
list(APPEND FLAGS_SUBPROJECT "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}")
list(APPEND FLAGS_SUBPROJECT "-DUSE_LTO=${USE_LTO}")

if(DEFINED CMAKE_TOOLCHAIN_FILE)
    list(APPEND FLAGS_SUBPROJECT "-DCMAKE_TOOLCHAIN_FILE=toolchains/${CMAKE_SYSTEM_PROCESSOR}.cmake")
endif()

if(DEFINED CMAKE_SYSTEM_SOC)
    string(TOUPPER "-DPLATFORM=${CMAKE_SYSTEM_SOC}" FLAGS_SUBPROJECT_HALM)
endif()

ExternalProject_Add(xcore
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/Libs/xcore
        CMAKE_COMMAND ${CMAKE_COMMAND}
        CMAKE_ARGS ${FLAGS_SUBPROJECT}
        BINARY_DIR ${CMAKE_BINARY_DIR}/xcore
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
)

ExternalProject_Add(halm
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/Libs/halm
        CMAKE_COMMAND ${CMAKE_COMMAND}
        CMAKE_ARGS
                ${FLAGS_SUBPROJECT}
                ${FLAGS_SUBPROJECT_HALM}
                -DCONFIG_FILE=${PROJECT_SOURCE_DIR}/Platform/${PLATFORM}/Halm.config
        BINARY_DIR ${CMAKE_BINARY_DIR}/halm
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
)

ExternalProject_Add(dpm
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/Libs/dpm
        CMAKE_COMMAND ${CMAKE_COMMAND}
        CMAKE_ARGS
                ${FLAGS_SUBPROJECT}
                ${FLAGS_SUBPROJECT_HALM}
        BINARY_DIR ${CMAKE_BINARY_DIR}/dpm
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
)

ExternalProject_Add(osw
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/Libs/osw
        CMAKE_COMMAND ${CMAKE_COMMAND}
        CMAKE_ARGS ${FLAGS_SUBPROJECT}
        BINARY_DIR ${CMAKE_BINARY_DIR}/osw
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
)

ExternalProject_Add(yaf
        SOURCE_DIR ${PROJECT_SOURCE_DIR}/Libs/yaf
        CMAKE_COMMAND ${CMAKE_COMMAND}
        CMAKE_ARGS
                ${FLAGS_SUBPROJECT}
                ${FLAGS_SUBPROJECT_YAF}
        BINARY_DIR ${CMAKE_BINARY_DIR}/yaf
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
)

set(PLATFORM_LIBS
        "${CMAKE_BINARY_DIR}/yaf/libyaf.a"
        "${CMAKE_BINARY_DIR}/osw/libosw.a"
        "${CMAKE_BINARY_DIR}/dpm/libdpm.a"
        "${CMAKE_BINARY_DIR}/halm/libhalm.a"
        "${CMAKE_BINARY_DIR}/xcore/libxcore.a"
        ${PLATFORM_LIBS}
)
