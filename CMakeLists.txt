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
option(BUILD_PROFILING "Enable MT_PROFILE_SCOPE/MT_PROFILE_FUNCTION instrumentation" ON)

find_package(SDL3 CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(efsw CONFIG REQUIRED)
find_package(EnTT CONFIG REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

# vcpkg's python3 port lays out its Python install differently to a normal one, entirely separate
# from wherever an embedding executable (MatchaEditor, Sandbox, either Tests target) ends up.
# Without pointing the embedded interpreter at the right directory as PYTHONHOME, CPython's own
# path autodetection - which assumes the running executable lives inside a normal Python install -
# fails to find the stdlib: on Windows this prints "Could not find platform independent libraries
# <prefix>" at startup; on Linux it fails harder, raising "failed to get the Python codec of the
# filesystem encoding" out of Py_InitializeFromConfig() itself (see PythonRuntime.cpp).
#
# The right directory differs by platform because the vcpkg port's own install layout does, per
# its portfile.cmake: on Windows, the stdlib (Lib/) is copied directly into tools/python3 alongside
# python.exe, so that directory alone is a valid home. On Linux, the autotools build installs the
# stdlib to lib/python3.<minor>/ under the triplet's install ROOT - a sibling of tools/, not nested
# inside it - so home has to be the install root itself for CPython's POSIX search (home/lib/
# python3.<minor>/os.py) to find anything.
if (WIN32)
    set(MATCHA_PYTHON_HOME "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/tools/python3" CACHE INTERNAL
        "Path to vcpkg's Python install, used as PYTHONHOME for the embedded interpreter")
else()
    set(MATCHA_PYTHON_HOME "${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}" CACHE INTERNAL
        "Path to vcpkg's Python install, used as PYTHONHOME for the embedded interpreter")
endif()

if (BUILD_EDITOR)
    # Must precede add_subdirectory(MatchaEditor) (and MatchaEngine, since it's added first below):
    # qt_standard_project_setup() sets CMAKE_AUTOMOC/CMAKE_AUTOUIC for this scope and below, and
    # does not apply retroactively to targets already defined. MatchaEngine itself no longer uses
    # Qt at all (see EngineViewportWidget in MatchaEditor for why) - this exists purely for
    # MatchaEditor, but is resolved here since AUTOMOC/AUTOUIC must be set before either
    # add_subdirectory() call.
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

    # GoogleTest 1.12.1's own build script (googletest/cmake/internal_utils.cmake) unconditionally
    # applies /W4 /WX to its own sources for any MSVC-family compiler, clang-cl included. Under
    # real cl.exe that's harmless - MSVC has no equivalent of clang's -Wcharacter-conversion - but
    # clang-cl does implement it, and gtest-printers.h's char32_t PrintTo() trips it under
    # -std=c++23 (this project's CMAKE_CXX_STANDARD). Suppressing just this one diagnostic here
    # (rather than disabling /WX entirely) keeps the rest of gtest's own warnings-as-errors policy
    # intact; this is gtest's own code, not this project's, so there's nothing to actually fix in it.
    if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        foreach (google_test_target IN ITEMS gtest gtest_main gmock gmock_main)
            target_compile_options(${google_test_target} PRIVATE -Wno-character-conversion)
        endforeach()
    endif()

    include(GoogleTest)
    enable_testing()
endif()

add_subdirectory(Vendor)
add_subdirectory(MatchaEngine)
add_subdirectory(Sandbox)

if (BUILD_EDITOR)
    add_subdirectory(MatchaEditor)
endif()