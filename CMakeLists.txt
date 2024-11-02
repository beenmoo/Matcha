cmake_minimum_required(VERSION 3.15)

include(GoogleTest)

project(Matcha)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Vendor)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_TESTS "Build the tests" ON)

add_subdirectory(Vendor)
add_subdirectory(MatchaEngine)
add_subdirectory(Sandbox)
add_subdirectory(MatchaEditor)

if (BUILD_TESTS)
    add_subdirectory(Tests)
endif()