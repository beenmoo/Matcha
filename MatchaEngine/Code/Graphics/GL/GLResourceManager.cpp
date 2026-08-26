#include "GLResourceManager.h"

#include <filesystem>
#include <unordered_set>

namespace Matcha
{
GLResourceManager::GLResourceManager()
{
    mFileWatcher.Watch();
}

ShaderHandle GLResourceManager::CreateShader(std::string_view name, const std::initializer_list<std::string>& paths)
{
    ShaderHandle handle(mNextShaderID++);
    mShaders.emplace(handle.GetID(), std::make_unique<GLShader>(name, paths));

    WatchShaderPaths(handle.GetID(), paths);

    return handle;
}

void GLResourceManager::DestroyShader(ShaderHandle handle)
{
    mShaders.erase(handle.GetID());

    std::scoped_lock lock(mShaderWatchMutex);

    for (auto& [path, shaderIDs] : mFileToShaderIDs)
        std::erase(shaderIDs, handle.GetID());
}

void GLResourceManager::ReloadModifiedShaders()
{
    std::vector<uint32_t> pending;

    {
        std::scoped_lock lock(mShaderWatchMutex);
        pending.swap(mPendingShaderReloads);
    }

    std::unordered_set<uint32_t> uniqueIDs(pending.begin(), pending.end());

    for (uint32_t id : uniqueIDs)
    {
        auto it = mShaders.find(id);

        if (it != mShaders.end())
            it->second->Reload();
    }
}

void GLResourceManager::WatchShaderPaths(uint32_t shaderID, const std::initializer_list<std::string>& paths)
{
    for (const auto& path : paths)
    {
        std::filesystem::path filePath(path);
        std::string directory = filePath.parent_path().string();
        std::string normalizedPath = filePath.lexically_normal().string();

        {
            std::scoped_lock lock(mShaderWatchMutex);
            mFileToShaderIDs[normalizedPath].push_back(shaderID);
        }

        if (mWatchedDirectories.contains(directory))
            continue;

        WatchHandle watchHandle = mFileWatcher.AddWatch(
            directory,
            [this](const std::string& dir, const std::string& filename, FileAction action)
            {
                if (action != FileAction::Modified)
                    return;

                std::string changedPath = (std::filesystem::path(dir) / filename).lexically_normal().string();

                std::scoped_lock lock(mShaderWatchMutex);

                auto it = mFileToShaderIDs.find(changedPath);

                if (it == mFileToShaderIDs.end())
                    return;

                mPendingShaderReloads.insert(mPendingShaderReloads.end(), it->second.begin(), it->second.end());
            },
            false);

        mWatchedDirectories.emplace(directory, watchHandle);
    }
}

TextureHandle GLResourceManager::CreateTexture(std::string_view path)
{
    TextureHandle handle(mNextTextureID++);
    mTextures.emplace(handle.GetID(), std::make_unique<GLTexture>(path));

    return handle;
}

TextureHandle GLResourceManager::CreateTexture(uint32_t width, uint32_t height)
{
    TextureHandle handle(mNextTextureID++);
    mTextures.emplace(handle.GetID(), std::make_unique<GLTexture>(width, height));

    return handle;
}

void GLResourceManager::DestroyTexture(TextureHandle handle)
{
    mTextures.erase(handle.GetID());
}

MeshHandle GLResourceManager::CreateMesh(std::span<const float> vertices,
                                         std::initializer_list<Utils::ShaderDataType> layout,
                                         std::span<const uint32_t> indices)
{
    auto mesh = std::make_unique<Mesh>();

    mesh->vertexBuffer = std::make_shared<GLVertexBuffer>(vertices.data(), static_cast<GLuint>(vertices.size_bytes()));
    mesh->vertexBuffer->SetLayout(std::make_shared<GLVertexBuffer::BufferLayout>(layout));

    mesh->indexBuffer = std::make_shared<GLIndexBuffer>(indices.data(), static_cast<GLuint>(indices.size()));

    mesh->vertexArray.AddVertexBuffer(mesh->vertexBuffer);
    mesh->vertexArray.SetIndexBuffer(mesh->indexBuffer);

    MeshHandle handle(mNextMeshID++);
    mMeshes.emplace(handle.GetID(), std::move(mesh));

    return handle;
}

void GLResourceManager::DestroyMesh(MeshHandle handle)
{
    mMeshes.erase(handle.GetID());
}

GLShader* GLResourceManager::GetShader(ShaderHandle handle)
{
    auto it = mShaders.find(handle.GetID());

    return it != mShaders.end() ? it->second.get() : nullptr;
}

GLTexture* GLResourceManager::GetTexture(TextureHandle handle)
{
    auto it = mTextures.find(handle.GetID());

    return it != mTextures.end() ? it->second.get() : nullptr;
}

GLResourceManager::Mesh* GLResourceManager::GetMesh(MeshHandle handle)
{
    auto it = mMeshes.find(handle.GetID());

    return it != mMeshes.end() ? it->second.get() : nullptr;
}
}  // namespace Matcha
