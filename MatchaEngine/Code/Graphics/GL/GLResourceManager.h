#pragma once

#include "Graphics/ResourceManager.h"
#include "GLMesh.h"
#include "GLShader.h"
#include "GLTexture.h"
#include "Graphics/ShaderDataType.h"
#include "Utils/FileWatcher.h"

#include <initializer_list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace Matcha
{
class GLResourceManager final : public ResourceManager
{
public:
    GLResourceManager();

    [[nodiscard]] ShaderHandle CreateShader(std::string_view name, const std::initializer_list<std::string>& paths) override;
    void DestroyShader(ShaderHandle handle) override;

    void ReloadModifiedShaders() override;

    [[nodiscard]] TextureHandle CreateTexture(std::string_view path) override;
    [[nodiscard]] TextureHandle CreateTexture(uint32_t width, uint32_t height) override;
    void DestroyTexture(TextureHandle handle) override;

    [[nodiscard]] MeshHandle CreateMesh(std::span<const float> vertices,
                                        std::initializer_list<ShaderDataType> layout,
                                        std::span<const uint32_t> indices) override;
    void DestroyMesh(MeshHandle handle) override;

    // Not part of the abstract ResourceManager interface: resolving a handle to the real
    // backend object is inherently backend-specific, so only GLRenderer (which knows it's
    // talking to a GLResourceManager) calls these.
    [[nodiscard]] GLShader* GetShader(ShaderHandle handle);
    [[nodiscard]] GLTexture* GetTexture(TextureHandle handle);
    [[nodiscard]] GLMesh* GetMesh(MeshHandle handle);

private:
    void WatchShaderPaths(uint32_t shaderID, const std::initializer_list<std::string>& paths);

private:
    uint32_t m_NextShaderID = 1;
    uint32_t m_NextTextureID = 1;
    uint32_t m_NextMeshID = 1;

    std::unordered_map<uint32_t, std::unique_ptr<GLShader>> m_Shaders;
    std::unordered_map<uint32_t, std::unique_ptr<GLTexture>> m_Textures;
    std::unordered_map<uint32_t, std::unique_ptr<GLMesh>> m_Meshes;

    // Shader hot-reload: efsw notifies on a background thread, so changed paths are only
    // recorded here and the actual GL reload happens on the main thread via ReloadModifiedShaders().
    FileWatcher m_FileWatcher;
    std::unordered_map<std::string, WatchHandle> m_WatchedDirectories;

    std::mutex m_ShaderWatchMutex;
    std::unordered_map<std::string, std::vector<uint32_t>> m_FileToShaderIDs;
    std::vector<uint32_t> m_PendingShaderReloads;
};
}  // namespace Matcha
