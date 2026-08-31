#pragma once

#include "Utility/FileWatcher.h"

#include <cstdint>
#include <initializer_list>
#include <mutex>
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
    // creation.
    void Watch(uint32_t shaderID, const std::initializer_list<std::string>& paths);

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
