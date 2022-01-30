# Copyright (C) 2018 xent
# Project is distributed under the terms of the GNU General Public License v3.0

include(FindPkgConfig)
pkg_check_modules(CHECK REQUIRED cppunit)

list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/Libs/xcore/cmake")

list_directories(TESTS_LIST "${CMAKE_CURRENT_SOURCE_DIR}")
list(REMOVE_ITEM TESTS_LIST "Shared")

file(GLOB_RECURSE TEST_SOURCES_SHARED
        "Shared/*.c"
        "Shared/*.cpp"
)
add_library(project_test_shared ${TEST_SOURCES_SHARED})
target_compile_options(project_test_shared PUBLIC --coverage)
target_include_directories(project_test_shared PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Shared")
target_link_libraries(project_test_shared PUBLIC project_core halm cppunit gcov)

foreach(TEST_NAME ${TESTS_LIST})
    file(GLOB_RECURSE TEST_SOURCES
            "${TEST_NAME}/*.c"
            "${TEST_NAME}/*.cpp"
    )
    add_executable(${TEST_NAME} ${TEST_SOURCES})
    add_test(${TEST_NAME} ${TEST_NAME})
    target_link_libraries(${TEST_NAME} PRIVATE project_test_shared)
endforeach()
