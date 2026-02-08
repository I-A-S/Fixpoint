cmake_minimum_required(VERSION 3.20)

project(Fixpoint)

if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    set(FIXPOINT_IS_TOP_LEVEL ON)
else()
    set(FIXPOINT_IS_TOP_LEVEL OFF)
endif()

set(FIXPOINT_ROOT "${CMAKE_CURRENT_LIST_DIR}" CACHE INTERNAL "")

if(TARGET IACrux)
    message(STATUS "Fixpoint: Using external IACrux provided by parent project")
else()
    message(STATUS "Fixpoint: Building standalone, using local IACrux submodule")
    if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/IACrux/CMakeLists.txt")
        message(FATAL_ERROR "IACrux submodule missing! Run: git submodule update --init --recursive")
    endif()
    add_subdirectory(IACrux)
endif()

crux_setup_project()

option(Fixpoint_BUILD_TESTS "Build unit tests" ${FIXPOINT_IS_TOP_LEVEL})

include(cmake/find_deps.cmake)

add_subdirectory(src/)

if(Fixpoint_BUILD_TESTS)
    add_subdirectory(tests)
endif()
