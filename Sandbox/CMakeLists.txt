set(TARGET_NAME Sandbox)

add_executable(${TARGET_NAME} Code/main.cpp)

add_subdirectory(Code/Core)

target_link_libraries(${TARGET_NAME} PRIVATE MatchaEngine)
target_include_directories(${TARGET_NAME} PRIVATE Code)

matcha_copy_engine_assets(${TARGET_NAME})

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

file(GLOB_RECURSE SHADER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders/*.frag")

target_sources(${TARGET_NAME} PRIVATE ${SHADER_SOURCES})
set_source_files_properties(${SHADER_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

if (MSVC)
    file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB_RECURSE HEADER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${CPP_SOURCES})
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_SOURCES})
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders" PREFIX "Shaders" FILES ${SHADER_SOURCES})
endif()

set_target_properties(
	${TARGET_NAME}
	PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Lib"
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Bin"
)