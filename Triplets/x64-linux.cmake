set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# CI only ever configures/builds Debug on Linux (see CMakePresets.json's
# "Linux Clang" preset) - skip vcpkg's unused Release variant to roughly
# halve from-scratch build time for large ports like Qt.
set(VCPKG_BUILD_TYPE debug)
