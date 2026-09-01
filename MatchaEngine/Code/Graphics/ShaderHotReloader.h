#pragma once

#include "Utility/FileWatcher.h"

#include <cstdint>
#include <mutex>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace Matcha
{
// Watches shader source files for changes and tracks which shader IDs need reloading as a
// result - split out of ResourceManager, which otherwise mixed this in with its actual job of
// owning Shader/Texture/Mesh resource lifetime.
class ShaderHotReloader
{
public:
    ShaderHotReloader();

    // Starts watching every path in `paths` on `shaderID`'s behalf - call once, at the shader's
    // creation. std::span rather than std::initializer_list so this accepts either a compile-time
    // {"a", "b"} list or a runtime-sized std::vector (e.g. from SceneSerializer, deserializing an
    // arbitrary number of shader stages) - every caller already has an already-typed, named
    // container by the time it reaches here, never a raw brace-literal argument, so span's
    // range constructor covers both without needing two overloads.
    void Watch(uint32_t shaderID, std::span<const std::string> paths);

    // Stops tracking `shaderID` - call when the shader itself is destroyed.
    void Forget(uint32_t shaderID);

    // Returns (and clears) every shader ID whose watched files changed since the last call -
    // efsw notifies on a background thread, so this just drains what accumulated there onto
    // whichever thread calls this. Intended to be polled once per frame.
    [[nodiscard]] std::vector<uint32_t> TakePendingReloads();

private:
    FileWatcher m_FileWatcher;
    std::unordered_map<std::string, WatchHandle> m_WatchedDirectories;

    std::mutex m_Mutex;
    std::unordered_map<std::string, std::vector<uint32_t>> m_FileToShaderIDs;
    std::vector<uint32_t> m_PendingReloads;
};
}  // namespace Matcha
