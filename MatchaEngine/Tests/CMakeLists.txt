set(TARGET_NAME MatchaEngineTests)

add_executable(${TARGET_NAME})

add_subdirectory(Code)

target_include_directories(${TARGET_NAME} PUBLIC Code)
target_precompile_headers(${TARGET_NAME} PRIVATE Code/pch.h)

# pybind11::embed is a PRIVATE dependency of MatchaEngine (doesn't propagate its include dirs/link
# requirements to consumers) - PythonRuntimeTests.cpp includes Scripting/PythonRuntime.h directly,
# so this target needs its own find_package/link for the same reason MatchaEditor or any other
# consumer of MatchaEngine would.
find_package(pybind11 CONFIG REQUIRED)

target_link_libraries(${TARGET_NAME} PRIVATE MatchaEngine gtest_main gmock_main pybind11::embed)

# Inherited from MatchaEngine/CMakeLists.txt's own add_compile_definitions() (this directory is
# added after it), but stated explicitly here so this target doesn't silently depend on where its
# add_subdirectory() call happens to sit in that file - glm's experimental headers hard-#error
# without it.
target_compile_definitions(${TARGET_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)

# Points SandboxScriptsTests.cpp at the real scripts Sandbox actually ships (Sandbox/Assets/Scripts),
# not a duplicated fixture copy - so the test proves the scripts players/Sandbox actually run work,
# not just a paraphrase of them that could silently drift out of sync.
target_compile_definitions(${TARGET_NAME} PRIVATE SANDBOX_SCRIPTS_DIR="${CMAKE_SOURCE_DIR}/Sandbox/Assets/Scripts")

gtest_discover_tests(${TARGET_NAME})

set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Tests")

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
