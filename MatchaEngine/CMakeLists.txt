set(TARGET_NAME MatchaEngine)

add_library(${TARGET_NAME} STATIC)

add_subdirectory(Code/Core)
add_subdirectory(Code/Graphics)
add_subdirectory(Code/Math)
add_subdirectory(Code/Utils)

target_include_directories(${TARGET_NAME} PUBLIC Code)
target_include_directories(${TARGET_NAME} PRIVATE ${Stb_INCLUDE_DIR})
target_precompile_headers(${TARGET_NAME} PRIVATE Code/pch.h)
target_link_libraries(${TARGET_NAME} PRIVATE SDL3::SDL3 efsw::efsw glm::glm spdlog::spdlog nlohmann_json::nlohmann_json glad assimp::assimp)

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
target_compile_definitions(${TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:MT_DEBUG=1>)

file(GLOB_RECURSE SHADER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders/*.frag")

target_sources(${TARGET_NAME} PRIVATE ${SHADER_SOURCES})
set_source_files_properties(${SHADER_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

# Path to MatchaEngine's own bundled assets, captured here (while CMAKE_CURRENT_SOURCE_DIR
# still points at MatchaEngine) so matcha_copy_engine_assets() resolves it correctly no
# matter which project's CMakeLists.txt calls it from.
set(MATCHA_ENGINE_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assets" CACHE INTERNAL "Path to MatchaEngine's bundled Assets directory")

# Copies MatchaEngine's Assets folder next to `target`'s built executable, and points Visual
# Studio's debugger at that same directory so F5 finds them too (its default working directory
# is the generated .vcxproj folder, not the executable's actual output directory).
function(matcha_copy_engine_assets target)
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${MATCHA_ENGINE_ASSETS_DIR}"
            "$<TARGET_FILE_DIR:${target}>/Assets"
        COMMENT "Copying MatchaEngine assets to ${target} output directory"
        VERBATIM)

    set_target_properties(${target} PROPERTIES
        VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${target}>")
endfunction()

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