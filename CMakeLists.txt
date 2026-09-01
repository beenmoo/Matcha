cmake_minimum_required(VERSION 4.2)

project(Matcha)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Vendor)

if (MSVC)
    # Keeps Code Analysis (including the editor's live background analysis) from
    # reporting on third-party headers reached via /external:I (vcpkg packages, glad).
    add_compile_options(/analyze:external-)
endif()

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)
option(BUILD_TESTS "Build the tests" ON)
option(BUILD_EDITOR "Build the Matcha Editor" ON)
option(BUILD_QT_BACKEND "Build the Qt Window/Input backend into MatchaEngine" ${BUILD_EDITOR})
option(BUILD_PROFILING "Enable MT_PROFILE_SCOPE/MT_PROFILE_FUNCTION instrumentation" ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

if (BUILD_QT_BACKEND)
    # Must precede add_subdirectory(MatchaEngine): qt_standard_project_setup() sets
    # CMAKE_AUTOMOC/CMAKE_AUTOUIC for this scope and below, and does not apply retroactively
    # to targets already defined.
    # Svg is only actually linked by MatchaEditor (Qt6::Svg, for Qt-Advanced-Docking-System's
    # icon-engine plugin - see MatchaEditor/CMakeLists.txt), not MatchaEngine's own Qt backend,
    # but every Qt component is found together here since this is the one place Qt6 is resolved.
    find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets Svg)
    qt_standard_project_setup()
endif()

add_subdirectory(Vendor)
add_subdirectory(MatchaEngine)
add_subdirectory(Sandbox)

if (BUILD_EDITOR)
    add_subdirectory(MatchaEditor)
endif()

if (BUILD_TESTS)
    add_subdirectory(Tests)
endif()