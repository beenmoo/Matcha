#pragma once

#include "Graphics/ShaderDataType.h"
#include "Mesh.h"
#include "RenderHandles.h"
#include "Shader.h"
#include "Texture.h"
#include "Utility/FileWatcher.h"

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace Matcha
{
class ResourceManager
{
public:
    ResourceManager();

    [[nodiscard]] ShaderHandle CreateShader(std::string_view name, const std::initializer_list<std::string>& paths);
    void DestroyShader(ShaderHandle handle);

    // Reloads any shaders whose source files changed on disk since the last call.
    // Intended to be called once per frame.
    void ReloadModifiedShaders();

    [[nodiscard]] TextureHandle CreateTexture(std::string_view path);
    [[nodiscard]] TextureHandle CreateTexture(uint32_t width, uint32_t height);
    void DestroyTexture(TextureHandle handle);

    [[nodiscard]] MeshHandle CreateMesh(std::span<const float> vertices,
                                        std::initializer_list<ShaderDataType> layout,
                                        std::span<const uint32_t> indices);
    void DestroyMesh(MeshHandle handle);

    [[nodiscard]] Shader* GetShader(ShaderHandle handle);
    [[nodiscard]] Texture* GetTexture(TextureHandle handle);
    [[nodiscard]] Mesh* GetMesh(MeshHandle handle);

private:
    void WatchShaderPaths(uint32_t shaderID, const std::initializer_list<std::string>& paths);

private:
    uint32_t m_NextShaderID = 1;
    uint32_t m_NextTextureID = 1;
    uint32_t m_NextMeshID = 1;

    std::unordered_map<uint32_t, std::unique_ptr<Shader>> m_Shaders;
    std::unordered_map<uint32_t, std::unique_ptr<Texture>> m_Textures;
    std::unordered_map<uint32_t, std::unique_ptr<Mesh>> m_Meshes;

    // Shader hot-reload: efsw notifies on a background thread, so changed paths are only
    // recorded here and the actual reload happens on the main thread via ReloadModifiedShaders().
    FileWatcher m_FileWatcher;
    std::unordered_map<std::string, WatchHandle> m_WatchedDirectories;

    std::mutex m_ShaderWatchMutex;
    std::unordered_map<std::string, std::vector<uint32_t>> m_FileToShaderIDs;
    std::vector<uint32_t> m_PendingShaderReloads;
};
}  // namespace Matcha
