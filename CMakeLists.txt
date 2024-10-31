cmake_minimum_required(VERSION 3.15)

project(Matcha)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_CXX_STANDARD 20)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Vendor)

add_subdirectory(Vendor)
add_subdirectory(MatchaEngine)
add_subdirectory(Sandbox)
add_subdirectory(MatchaEditor)