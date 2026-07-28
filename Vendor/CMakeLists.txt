cmake_minimum_required(VERSION 3.15)

include(FetchContent)

add_subdirectory(glad)
add_subdirectory(stb_image)

set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TESTS OFF)

find_package(SDL3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)

if (BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    
    set(SDL_TESTS ON)
endif()