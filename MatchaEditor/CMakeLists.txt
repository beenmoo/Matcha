set(TARGET_NAME MatchaEditor)

add_executable(${TARGET_NAME} Code/main.cpp)

add_subdirectory(Code/Core)
add_subdirectory(Code/Panels)
add_subdirectory(Code/Widgets)
add_subdirectory(Code/Utility)

# spdlog::spdlog (not just MatchaEngine, which only links it PRIVATE) is needed directly here:
# ConsoleSink.cpp includes spdlog headers itself (to subclass base_sink), and without spdlog's
# own target propagating its INTERFACE_COMPILE_DEFINITIONS (SPDLOG_COMPILED_LIB/SPDLOG_SHARED_LIB,
# since vcpkg builds it as spdlogd.dll here) those headers fall back to embedding full symbol
# definitions in this target's objects, which then collide with the DLL's exported ones at link time.
target_link_libraries(${TARGET_NAME} PRIVATE MatchaEngine Qt6::Widgets Qt6::OpenGLWidgets spdlog::spdlog)
target_include_directories(${TARGET_NAME} PRIVATE Code)
target_precompile_headers(${TARGET_NAME} PRIVATE Code/pch.h)

matcha_copy_engine_assets(${TARGET_NAME})

# Copies MatchaEditor's own Assets (editor-only shaders, icons, etc.) into the same output
# Assets/ directory that matcha_copy_engine_assets() populates with the engine's shaders.
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/Assets"
        "$<TARGET_FILE_DIR:${TARGET_NAME}>/Assets"
    COMMENT "Copying MatchaEditor assets to ${TARGET_NAME} output directory"
    VERBATIM)

if (UNIX AND NOT APPLE)
    # vcpkg's x64-linux triplet is static (VCPKG_LIBRARY_LINKAGE static), so Qt's platform
    # plugins build as static libraries (libqxcb.a/libqwayland.a) instead of loadable .so
    # modules - there's nothing on disk for Qt's normal runtime plugin-scanning to find, unlike
    # the WIN32 branch below where the vcpkg triplet is dynamic and qwindows.dll is a real
    # runtime-loaded plugin. A static build has no plugin loader at all, so the platform plugin
    # has to be linked directly into the executable and registered at compile time instead.
    # An explicit INCLUDE list replaces qt_import_plugins()'s default behavior entirely rather
    # than adding to it, so the GL-integration plugins (needed for QOpenGLWidget to get a real
    # context under each platform plugin - EGL-over-XCB and EGL-over-Wayland respectively, since
    # this qtbase build has no GLX) have to be listed explicitly too, not just the platform
    # plugins themselves.
    qt_import_plugins(${TARGET_NAME}
        INCLUDE
            Qt6::QXcbIntegrationPlugin
            Qt6::QXcbEglIntegrationPlugin
            Qt6::QWaylandIntegrationPlugin
            Qt6::QWaylandEglClientBufferPlugin)
endif()

if (WIN32)
    qt_generate_deploy_app_script(
        TARGET ${TARGET_NAME}
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR)
    install(SCRIPT ${deploy_script})

    # qt_generate_deploy_app_script() only runs at `cmake --install` time, which nobody runs
    # during normal iteration - without the platform plugin (qwindows[d].dll) next to the exe,
    # QApplication can't even construct. Copy it directly as part of every normal build too, so
    # F5/cmake --build just works.
    add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${TARGET_NAME}>/platforms"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}/$<$<CONFIG:Debug>:debug/>Qt6/plugins/platforms/qwindows$<$<CONFIG:Debug>:d>.dll"
            "$<TARGET_FILE_DIR:${TARGET_NAME}>/platforms/"
        COMMENT "Copying Qt platform plugin (qwindows)"
        VERBATIM)
endif()

add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

file(GLOB_RECURSE SHADER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders/*.vert"
    "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders/*.frag")

target_sources(${TARGET_NAME} PRIVATE ${SHADER_SOURCES})
set_source_files_properties(${SHADER_SOURCES} PROPERTIES HEADER_FILE_ONLY TRUE)

if (MSVC)
    file(GLOB_RECURSE CPP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB_RECURSE HEADER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
    file(GLOB_RECURSE CMAKE_LISTS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt")
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${CPP_SOURCES})
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_SOURCES})
    source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Assets/Shaders" PREFIX "Shaders" FILES ${SHADER_SOURCES})
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