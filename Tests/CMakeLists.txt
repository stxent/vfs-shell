# Copyright (C) 2018 xent
# Project is distributed under the terms of the GNU General Public License v3.0

add_definitions(-D_POSIX_C_SOURCE=200809L)

function(list_directories _search_results _directory_path)
    file(GLOB _entry_list RELATIVE "${_directory_path}" "${_directory_path}/*")
    set(_directory_list "")
    foreach(_entry ${_entry_list})
        if(IS_DIRECTORY "${_directory_path}/${_entry}")
            list(APPEND _directory_list "${_entry}")
        endif()
    endforeach()
    set(${_search_results} ${_directory_list} PARENT_SCOPE)
endfunction()

list_directories(TESTS_LIST "${CMAKE_CURRENT_SOURCE_DIR}")

foreach(TEST_NAME ${TESTS_LIST})
    file(GLOB_RECURSE TEST_SOURCES "${TEST_NAME}/*.cpp")
    add_executable(${TEST_NAME} ${TEST_SOURCES})
    add_test(${TEST_NAME} ${TEST_NAME})
    target_compile_options(${TEST_NAME} PRIVATE --coverage)
    target_link_libraries(${TEST_NAME} PRIVATE halm osw yaf check gcov)
endforeach()
