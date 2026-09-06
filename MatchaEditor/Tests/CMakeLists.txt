set(TARGET_NAME MatchaEditorTests)

add_executable(${TARGET_NAME})

add_subdirectory(Code)

# MatchaEditor is an executable (add_executable in MatchaEditor/CMakeLists.txt), not a library, so
# there's nothing to target_link_libraries() against here the way this target links MatchaEngine -
# the editor sources under test are compiled straight into this binary instead (see Code/
# CMakeLists.txt). That's only viable for editor code with no Qt dependency; everything Qt-bound
# (panels, widgets, EditorMainWindow) stays out of reach until MatchaEditor grows a library target.
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/MatchaEditor/Code)
target_include_directories(${TARGET_NAME} PUBLIC Code)

# MatchaEngine's own test doubles (NullRendererAPI) are reused rather than duplicated here:
# MatchaEditor already depends on MatchaEngine, so its tests depending on MatchaEngine's test
# support headers runs in the same direction, not against it.
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/MatchaEngine/Tests/Code)

target_precompile_headers(${TARGET_NAME} PRIVATE Code/pch.h)

# Same reasoning as MatchaEditor's own link line: pybind11::embed is PRIVATE on MatchaEngine, and
# ComponentSnapshotTests.cpp includes Scene/Component/PythonScriptComponent.h (and PythonRuntime.h)
# directly, so pybind11's headers have to be findable in this target's own compilation.
find_package(pybind11 CONFIG REQUIRED)

target_link_libraries(${TARGET_NAME} PRIVATE MatchaEngine gtest_main gmock_main pybind11::embed)

# Inherited from MatchaEditor/CMakeLists.txt's own add_compile_definitions() (this directory is
# added after it), but stated explicitly so this target doesn't silently depend on where its
# add_subdirectory() call happens to sit in that file - glm's experimental headers hard-#error
# without it.
target_compile_definitions(${TARGET_NAME} PRIVATE GLM_ENABLE_EXPERIMENTAL)

# ComponentSnapshotTests.cpp restores a PythonScriptComponent snapshot and then actually runs the
# restored bindings, using the same real Sandbox scripts MatchaEngineTests does - a snapshot that
# round-trips as JSON but produces bindings the runtime can't instantiate isn't a working undo.
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
