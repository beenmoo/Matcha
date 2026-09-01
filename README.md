# Matcha

Matcha is a C++23 game engine built on SDL3 and OpenGL, with an Entity-Component-System scene layer (EnTT), a Qt6-based editor ("Matcha Editor"), and a sandbox application for trying out engine features directly.

## Status

Core systems (windowing, input, time, logging, application lifecycle) and the OpenGL graphics primitives (vertex/index buffers, vertex arrays, shaders, textures, framebuffers, uniform buffers) are in place, wired up through a `RendererAPI`/`Renderer` abstraction layer designed to eventually support additional backends (Vulkan/DirectX12) alongside OpenGL. On top of that sits an EnTT-backed scene graph (entities, transform/camera/light/material/mesh/hierarchy/native-script components) driven each frame by a set of systems (Transform, Camera, Light, Render, Script). Shaders hot-reload on save (`efsw` file watching), and models load via `assimp`. Matcha Editor is a working Qt6 Widgets application with a scene hierarchy panel, an inspector panel, a console panel, and a 3D viewport with a fly camera (WASD + hold-right-mouse-to-look).

## Project Structure

| Directory | Description |
| --- | --- |
| `MatchaEngine` | Core engine library: application/window/input lifecycle, OpenGL graphics primitives, the ECS scene layer, math types, model loading. |
| `MatchaEditor` | Matcha Editor — the Qt6 Widgets-based editor application. |
| `Sandbox` | Minimal application for testing engine features directly, including sample assets/shaders and native-script demo entities (camera controller, flashlight, rotation). |
| `Tests` | Engine unit tests (GoogleTest). |
| `Vendor` | Vendored third-party sources built in-tree (currently `glad`). |
| `vcpkg` | vcpkg submodule, used in manifest mode to resolve the remaining dependencies. |

## Dependencies

Resolved automatically via [vcpkg](https://github.com/microsoft/vcpkg) manifest mode (`vcpkg.json`):

- [SDL3](https://www.libsdl.org/) — windowing, input, GL context creation (Sandbox's window backend)
- [glad](https://glad.dav1d.de/) — OpenGL function loading (vendored)
- [glm](https://github.com/g-truc/glm) — math
- [spdlog](https://github.com/gabime/spdlog) — logging
- [nlohmann-json](https://github.com/nlohmann/json) — JSON
- [stb](https://github.com/nothings/stb) — image loading
- [assimp](https://github.com/assimp/assimp) — model importing
- [efsw](https://github.com/SpartanJ/efsw) — filesystem watching, used to hot-reload shaders on save
- [EnTT](https://github.com/skypjack/entt) — the ECS backing `Scene`/`Entity`/components
- [pybind11](https://github.com/pybind/pybind11) — embedded Python interpreter (linked; not yet wired into any engine subsystem — current scripting is native C++ via `NativeScriptComponent`)
- [Qt6](https://www.qt.io/) (`qtbase`, Widgets + OpenGLWidgets) — Matcha Editor's UI toolkit and window/input backend, when `BUILD_QT_BACKEND` is on
- [imgui](https://github.com/ocornut/imgui) — vendored dependency, not currently used by any target

## Prerequisites

- CMake 4.2+
- Ninja (Linux) or Visual Studio 2026 (Windows)
- A C++23 compiler:
  - Linux: Clang with `clang-scan-deps` matching the compiler version (only needed if C++ module dependency scanning is enabled; this project builds with `CMAKE_CXX_SCAN_FOR_MODULES` off)
  - Windows: MSVC via Visual Studio 18 2026
- On Linux, dev packages for SDL3's video backends, Qt's platform plugins, and font rendering:
  ```sh
  sudo apt install libx11-dev libxft-dev libxext-dev libxrandr-dev libxinerama-dev \
      libxcursor-dev libxi-dev libwayland-dev libxkbcommon-dev libegl1-mesa-dev \
      wayland-protocols libx11-xcb-dev libglu1-mesa-dev libxrender-dev \
      libxkbcommon-x11-dev fontconfig fonts-dejavu $(apt-cache search '^libxcb.*-dev' | awk '{print $1}')
  ```
  vcpkg builds Qt itself from source on Linux (static triplet), so these need to be present on the system before the first configure — Qt's `xcb`/`wayland`/`egl` feature checks fail without them, and the build won't retry them until the vcpkg buildtree for `qtbase` is cleared (`rm -rf vcpkg/buildtrees/qtbase`) even after installing what was missing.

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

The first configure will bootstrap vcpkg and build all manifest dependencies, which can take a while — Qt itself is built from source on Linux, so expect the first `qtbase` build specifically to take several minutes on its own.

Build outputs are placed under each subproject's `Bin` directory inside the build tree, e.g. `Build/Clang/Sandbox/Bin/Sandbox`, `Build/Clang/MatchaEditor/Bin/MatchaEditor`.

### Running Matcha Editor under WSL/WSLg

If a window opens but stays blank/unresponsive, WSLg's Wayland socket lives outside the standard `XDG_RUNTIME_DIR`, which trips up Qt's Wayland platform plugin. Either launch with the override, or let Qt fall back to the xcb (Xwayland) platform plugin instead — both work:

```sh
XDG_RUNTIME_DIR=/mnt/wslg/runtime-dir ./Build/Clang/MatchaEditor/Bin/MatchaEditor
```

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
| `BUILD_EDITOR` | `ON` | Build the `MatchaEditor` target. |
| `BUILD_QT_BACKEND` | `BUILD_EDITOR` | Build the Qt window/input backend into `MatchaEngine` (required by `MatchaEditor`; `Sandbox` uses the SDL3 backend regardless). |
| `BUILD_PROFILING` | `ON` | Enable `MT_PROFILE_SCOPE`/`MT_PROFILE_FUNCTION` instrumentation. |
