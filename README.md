# Matcha

Matcha is a C++23 game engine built on SDL3 and OpenGL, with an ImGui-based editor ("Hazelnut") and a sandbox application for trying out engine features in isolation.

## Status

Early development. Core systems (windowing, input, time, logging, application lifecycle) and the low-level OpenGL primitives (vertex/index buffers, vertex arrays, shaders, textures, framebuffers) are in place; the higher-level renderer that ties them together into actual on-screen output is still being built out.

## Project Structure

| Directory | Description |
| --- | --- |
| `MatchaEngine` | Core engine library: application/window/input lifecycle, OpenGL graphics primitives, math types. |
| `MatchaEditor` | "Hazelnut" — the ImGui-based editor application. |
| `Sandbox` | Minimal application for testing engine features directly, including sample assets/shaders. |
| `Tests` | Engine unit tests (GoogleTest). |
| `Vendor` | Vendored third-party sources built in-tree (currently `glad`). |
| `vcpkg` | vcpkg submodule, used in manifest mode to resolve the remaining dependencies. |

## Dependencies

Resolved automatically via [vcpkg](https://github.com/microsoft/vcpkg) manifest mode (`vcpkg.json`):

- [SDL3](https://www.libsdl.org/) — windowing, input, GL context creation
- [glad](https://glad.dav1d.de/) — OpenGL function loading (vendored)
- [glm](https://github.com/g-truc/glm) — math
- [spdlog](https://github.com/gabime/spdlog) — logging
- [nlohmann-json](https://github.com/nlohmann/json) — JSON
- [imgui](https://github.com/ocornut/imgui) — editor UI
- [stb](https://github.com/nothings/stb) — image loading
- [assimp](https://github.com/assimp/assimp) — model importing

## Prerequisites

- CMake 4.2+
- Ninja (Linux) or Visual Studio 2026 (Windows)
- A C++23 compiler:
  - Linux: Clang with `clang-scan-deps` matching the compiler version (only needed if C++ module dependency scanning is enabled; this project builds with `CMAKE_CXX_SCAN_FOR_MODULES` off)
  - Windows: MSVC via Visual Studio 18 2026
- On Linux, X11/Wayland dev packages if you want those SDL3 video backends: `sudo apt install libx11-dev libxft-dev libxext-dev libwayland-dev libxkbcommon-dev libegl1-mesa-dev`

## Building

Clone with submodules (vcpkg is a submodule):

```sh
git clone --recurse-submodules https://github.com/beenmoo/Matcha.git
cd Matcha
```

Configure and build using the CMake preset for your platform:

```sh
# Linux (Clang + Ninja)
cmake --preset "Linux Clang"
cmake --build Build/Clang

# Windows (Visual Studio)
cmake --preset "Windows Visual Studio"
cmake --build Build/VS
```

The first configure will bootstrap vcpkg and build all manifest dependencies, which can take several minutes.

Build outputs are placed under each subproject's `Bin` directory inside the build tree, e.g. `Build/Clang/Sandbox/Bin/Sandbox`.

## Running Tests

Tests are built by default (`BUILD_TESTS`) and can be run via the generated `Tests` binary or through CTest from the build directory:

```sh
cd Build/Clang
ctest
```

## CMake Options

| Option | Default | Description |
| --- | --- | --- |
| `BUILD_SHARED_LIBS` | `OFF` | Build vendored/engine libraries as shared instead of static. |
| `BUILD_TESTS` | `ON` | Build the `Tests` target. |
| `BUILD_EDITOR` | `ON` | Build the `MatchaEditor` ("Hazelnut") target. |
