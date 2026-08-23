cmake_minimum_required(VERSION 3.15)

set(TARGET_NAME Sandbox)

add_executable(${TARGET_NAME} Code/main.cpp)

add_subdirectory(Code/Core)

target_link_libraries(${TARGET_NAME} PRIVATE MatchaEngine)
target_include_directories(${TARGET_NAME} PRIVATE Code)

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