cmake_minimum_required(VERSION 4.2)

project(Matcha)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(VENDOR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Vendor)

if (MSVC AND NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # Keeps Code Analysis (including the editor's live background analysis) from
    # reporting on third-party headers reached via /external:I (vcpkg packages, glad).
    # clang-cl also sets MSVC (it's MSVC-ABI-compatible) but doesn't understand this
    # MSVC-Code-Analysis-only flag, hence the extra compiler-id check.
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

if (BUILD_TESTS)
    # GoogleTest is fetched once here rather than from either module's Tests/CMakeLists.txt:
    # MatchaEngine/Tests and MatchaEditor/Tests both need it, and FetchContent_MakeAvailable can
    # only define the gtest/gmock targets once per build. Has to precede the add_subdirectory()
    # calls below, since each module now pulls in its own Tests subdirectory.
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1)
    FetchContent_MakeAvailable(googletest)
    set_target_properties(gtest gtest_main gmock gmock_main PROPERTIES FOLDER "ThirdParty")

    include(GoogleTest)
    enable_testing()
endif()

add_subdirectory(Vendor)
add_subdirectory(MatchaEngine)
add_subdirectory(Sandbox)

if (BUILD_EDITOR)
    add_subdirectory(MatchaEditor)
endif()