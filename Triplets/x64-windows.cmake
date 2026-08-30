set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE dynamic)
set(VCPKG_PROVIDED_FORTRAN ON)

# CI only ever builds/tests Debug (see Build.yaml) - skip vcpkg's unused
# Release variant to roughly halve from-scratch build time for large ports
# like Qt.
set(VCPKG_BUILD_TYPE debug)
