cmake_minimum_required(VERSION 3.15)

project(MatchaEngine)

add_library(${PROJECT_NAME} STATIC)

add_subdirectory(Code/Core)
add_subdirectory(Code/Graphics)
add_subdirectory(Code/Math)
add_subdirectory(Code/Utils)

target_include_directories(${PROJECT_NAME} PUBLIC Code ${VENDOR_DIR})
target_precompile_headers(${PROJECT_NAME} PRIVATE Code/pch.h)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3 glm::glm spdlog::spdlog nlohmann_json::nlohmann_json stb_image glad)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES ${SOURCES})
endif()

set_target_properties(
    ${PROJECT_NAME} 
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/Bin"
)