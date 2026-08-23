include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1)
FetchContent_MakeAvailable(googletest)
set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER "ThirdParty")

enable_testing()

set(TARGET_NAME Tests)

add_executable(${TARGET_NAME} Code/test.cpp)
add_test(NAME ${TARGET_NAME} COMMAND ${TARGET_NAME})

target_include_directories(${TARGET_NAME} PUBLIC Code)
target_precompile_headers(${TARGET_NAME} PRIVATE Code/pch.h)
target_link_libraries(${TARGET_NAME} PRIVATE MatchaEngine gtest_main gmock_main)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

if (MSVC)
    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES ${SOURCES})
endif()

set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Bin"
)