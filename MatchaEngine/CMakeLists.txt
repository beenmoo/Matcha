set(TARGET_NAME MatchaEngine)

add_library(${TARGET_NAME} STATIC)

add_subdirectory(Code/Core)
add_subdirectory(Code/Graphics)
add_subdirectory(Code/Math)
add_subdirectory(Code/Utils)

target_include_directories(${TARGET_NAME} PUBLIC Code)
target_include_directories(${TARGET_NAME} PRIVATE ${Stb_INCLUDE_DIR})
target_precompile_headers(${TARGET_NAME} PRIVATE Code/pch.h)
target_link_libraries(${TARGET_NAME} PUBLIC SDL3::SDL3 glm::glm spdlog::spdlog nlohmann_json::nlohmann_json glad)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:MT_DEBUG=1>)

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