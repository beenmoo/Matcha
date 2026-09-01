#include "ShaderHotReloader.h"

#include <filesystem>

namespace Matcha
{
ShaderHotReloader::ShaderHotReloader()
{
    m_FileWatcher.Watch();
}

void ShaderHotReloader::Watch(uint32_t shaderID, std::span<const std::string> paths)
{
    for (const auto& path : paths)
    {
        std::filesystem::path filePath(path);
        std::string directory = filePath.parent_path().string();
        std::string normalizedPath = filePath.lexically_normal().string();

        {
            std::scoped_lock lock(m_Mutex);
            m_FileToShaderIDs[normalizedPath].push_back(shaderID);
        }

        if (m_WatchedDirectories.contains(directory))
            continue;

        WatchHandle watchHandle = m_FileWatcher.AddWatch(
            directory,
            [this](const std::string& dir, const std::string& filename, FileAction action) {
                if (action != FileAction::Modified)
                    return;

                std::string changedPath = (std::filesystem::path(dir) / filename).lexically_normal().string();

                std::scoped_lock lock(m_Mutex);

                auto it = m_FileToShaderIDs.find(changedPath);

                if (it == m_FileToShaderIDs.end())
                    return;

                m_PendingReloads.insert(m_PendingReloads.end(), it->second.begin(), it->second.end());
            },
            false);

        m_WatchedDirectories.emplace(directory, watchHandle);
    }
}

void ShaderHotReloader::Forget(uint32_t shaderID)
{
    std::scoped_lock lock(m_Mutex);

    for (auto& [path, shaderIDs] : m_FileToShaderIDs)
        std::erase(shaderIDs, shaderID);
}

std::vector<uint32_t> ShaderHotReloader::TakePendingReloads()
{
    std::vector<uint32_t> pending;

    std::scoped_lock lock(m_Mutex);
    pending.swap(m_PendingReloads);

    return pending;
}
}  // namespace Matcha
