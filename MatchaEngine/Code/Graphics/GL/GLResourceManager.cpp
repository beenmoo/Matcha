#include "GLResourceManager.h"

#include <filesystem>
#include <unordered_set>

namespace Matcha
{
GLResourceManager::GLResourceManager()
{
    m_FileWatcher.Watch();
}

ShaderHandle GLResourceManager::CreateShader(std::string_view name, const std::initializer_list<std::string>& paths)
{
    ShaderHandle handle(m_NextShaderID++);
    m_Shaders.emplace(handle.GetID(), std::make_unique<GLShader>(name, paths));

    WatchShaderPaths(handle.GetID(), paths);

    return handle;
}

void GLResourceManager::DestroyShader(ShaderHandle handle)
{
    m_Shaders.erase(handle.GetID());

    std::scoped_lock lock(m_ShaderWatchMutex);

    for (auto& [path, shaderIDs] : m_FileToShaderIDs)
        std::erase(shaderIDs, handle.GetID());
}

void GLResourceManager::ReloadModifiedShaders()
{
    std::vector<uint32_t> pending;

    {
        std::scoped_lock lock(m_ShaderWatchMutex);
        pending.swap(m_PendingShaderReloads);
    }

    std::unordered_set<uint32_t> uniqueIDs(pending.begin(), pending.end());

    for (uint32_t id : uniqueIDs)
    {
        auto it = m_Shaders.find(id);

        if (it != m_Shaders.end())
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
            std::scoped_lock lock(m_ShaderWatchMutex);
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

                std::scoped_lock lock(m_ShaderWatchMutex);

                auto it = m_FileToShaderIDs.find(changedPath);

                if (it == m_FileToShaderIDs.end())
                    return;

                m_PendingShaderReloads.insert(m_PendingShaderReloads.end(), it->second.begin(), it->second.end());
            },
            false);

        m_WatchedDirectories.emplace(directory, watchHandle);
    }
}

TextureHandle GLResourceManager::CreateTexture(std::string_view path)
{
    TextureHandle handle(m_NextTextureID++);
    m_Textures.emplace(handle.GetID(), std::make_unique<GLTexture>(path));

    return handle;
}

TextureHandle GLResourceManager::CreateTexture(uint32_t width, uint32_t height)
{
    TextureHandle handle(m_NextTextureID++);
    m_Textures.emplace(handle.GetID(), std::make_unique<GLTexture>(width, height));

    return handle;
}

void GLResourceManager::DestroyTexture(TextureHandle handle)
{
    m_Textures.erase(handle.GetID());
}

MeshHandle GLResourceManager::CreateMesh(std::span<const float> vertices,
                                         std::initializer_list<ShaderDataType> layout,
                                         std::span<const uint32_t> indices)
{
    auto mesh = std::make_unique<Mesh>();

    mesh->vertexBuffer = std::make_shared<GLVertexBuffer>(vertices.data(), static_cast<GLuint>(vertices.size_bytes()));
    mesh->vertexBuffer->SetLayout(std::make_shared<BufferLayout>(layout));

    mesh->indexBuffer = std::make_shared<GLIndexBuffer>(indices.data(), static_cast<GLuint>(indices.size()));

    mesh->vertexArray.AddVertexBuffer(mesh->vertexBuffer);
    mesh->vertexArray.SetIndexBuffer(mesh->indexBuffer);

    MeshHandle handle(m_NextMeshID++);
    m_Meshes.emplace(handle.GetID(), std::move(mesh));

    return handle;
}

void GLResourceManager::DestroyMesh(MeshHandle handle)
{
    m_Meshes.erase(handle.GetID());
}

GLShader* GLResourceManager::GetShader(ShaderHandle handle)
{
    auto it = m_Shaders.find(handle.GetID());

    return it != m_Shaders.end() ? it->second.get() : nullptr;
}

GLTexture* GLResourceManager::GetTexture(TextureHandle handle)
{
    auto it = m_Textures.find(handle.GetID());

    return it != m_Textures.end() ? it->second.get() : nullptr;
}

GLResourceManager::Mesh* GLResourceManager::GetMesh(MeshHandle handle)
{
    auto it = m_Meshes.find(handle.GetID());

    return it != m_Meshes.end() ? it->second.get() : nullptr;
}
}  // namespace Matcha
