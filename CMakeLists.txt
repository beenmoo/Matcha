cmake_minimum_required(VERSION 4.2)

project(Matcha)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Vendor)

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_TESTS "Build the tests" ON)
option(BUILD_EDITOR "Build the Matcha Editor" ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)

add_subdirectory(Vendor)
add_subdirectory(MatchaEngine)
add_subdirectory(Sandbox)

if (BUILD_EDITOR)
    add_subdirectory(MatchaEditor)
endif()

if (BUILD_TESTS)
    add_subdirectory(Tests)
endif()