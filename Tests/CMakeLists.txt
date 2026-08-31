include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1)
FetchContent_MakeAvailable(googletest)
set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER "ThirdParty")

include(GoogleTest)
enable_testing()

set(TARGET_NAME Tests)

add_executable(${TARGET_NAME}
    Code/CameraSystemTests.cpp
    Code/FileWatcherTests.cpp
    Code/HierarchyComponentTests.cpp
    Code/MatrixTests.cpp
    Code/QuaternionTests.cpp
    Code/RendererTests.cpp
    Code/SceneTests.cpp
    Code/TransformSystemTests.cpp
    Code/TransformTests.cpp
    Code/VectorTests.cpp)

target_include_directories(${TARGET_NAME} PUBLIC Code)
target_precompile_headers(${TARGET_NAME} PRIVATE Code/pch.h)
target_link_libraries(${TARGET_NAME} PRIVATE MatchaEngine gtest_main gmock_main)

gtest_discover_tests(${TARGET_NAME})

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

if (MSVC)
    file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB_RECURSE HEADER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    file(GLOB_RECURSE CMAKE_LISTS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${CPP_SOURCES})
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_SOURCES})
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "CMake Files" FILES ${CMAKE_LISTS_SOURCES})

    target_sources(${TARGET_NAME} PRIVATE ${CMAKE_LISTS_SOURCES})
    set_source_files_properties(${CMAKE_LISTS_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)
endif()

set_target_properties(
    ${TARGET_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Bin"
)