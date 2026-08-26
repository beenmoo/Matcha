#pragma once

#include "Graphics/ResourceManager.h"
#include "GLIndexBuffer.h"
#include "GLShader.h"
#include "GLTexture.h"
#include "GLVertexArray.h"
#include "GLVertexBuffer.h"
#include "Utils/FileWatcher.h"
#include "Utils/ShaderDataType.h"

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
    struct Mesh
    {
        GLVertexArray vertexArray;
        std::shared_ptr<GLVertexBuffer> vertexBuffer;
        std::shared_ptr<GLIndexBuffer> indexBuffer;
    };

public:
    GLResourceManager();

    [[nodiscard]] ShaderHandle CreateShader(std::string_view name, const std::initializer_list<std::string>& paths) override;
    void DestroyShader(ShaderHandle handle) override;

    void ReloadModifiedShaders() override;

    [[nodiscard]] TextureHandle CreateTexture(std::string_view path) override;
    [[nodiscard]] TextureHandle CreateTexture(uint32_t width, uint32_t height) override;
    void DestroyTexture(TextureHandle handle) override;

    [[nodiscard]] MeshHandle CreateMesh(std::span<const float> vertices,
                                        std::initializer_list<Utils::ShaderDataType> layout,
                                        std::span<const uint32_t> indices) override;
    void DestroyMesh(MeshHandle handle) override;

    // Not part of the abstract ResourceManager interface: resolving a handle to the real
    // backend object is inherently backend-specific, so only GLRenderer (which knows it's
    // talking to a GLResourceManager) calls these.
    [[nodiscard]] GLShader* GetShader(ShaderHandle handle);
    [[nodiscard]] GLTexture* GetTexture(TextureHandle handle);
    [[nodiscard]] Mesh* GetMesh(MeshHandle handle);

private:
    void WatchShaderPaths(uint32_t shaderID, const std::initializer_list<std::string>& paths);

private:
    uint32_t mNextShaderID = 1;
    uint32_t mNextTextureID = 1;
    uint32_t mNextMeshID = 1;

    std::unordered_map<uint32_t, std::unique_ptr<GLShader>> mShaders;
    std::unordered_map<uint32_t, std::unique_ptr<GLTexture>> mTextures;
    std::unordered_map<uint32_t, std::unique_ptr<Mesh>> mMeshes;

    // Shader hot-reload: efsw notifies on a background thread, so changed paths are only
    // recorded here and the actual GL reload happens on the main thread via ReloadModifiedShaders().
    FileWatcher mFileWatcher;
    std::unordered_map<std::string, WatchHandle> mWatchedDirectories;

    std::mutex mShaderWatchMutex;
    std::unordered_map<std::string, std::vector<uint32_t>> mFileToShaderIDs;
    std::vector<uint32_t> mPendingShaderReloads;
};
}  // namespace Matcha
