cmake_minimum_required(VERSION 3.15)

include(FetchContent)

add_subdirectory(glad)
add_subdirectory(stb_image)

if (BUILD_TESTS)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
endif()