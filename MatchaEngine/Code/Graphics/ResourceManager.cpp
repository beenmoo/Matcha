#include "ResourceManager.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

#include <filesystem>
#include <unordered_set>

namespace Matcha
{
ShaderHandle ResourceManager::CreateShader(std::string_view name, const std::initializer_list<std::string>& paths)
{
    ShaderHandle handle(m_NextShaderID++);
    m_Shaders.emplace(handle.GetID(), Shader::Create(name, paths));

    m_ShaderHotReloader.Watch(handle.GetID(), paths);

    return handle;
}

void ResourceManager::DestroyShader(ShaderHandle handle)
{
    m_Shaders.erase(handle.GetID());
    m_ShaderHotReloader.Forget(handle.GetID());
}

void ResourceManager::ReloadModifiedShaders()
{
    std::vector<uint32_t> pending = m_ShaderHotReloader.TakePendingReloads();
    std::unordered_set<uint32_t> uniqueIDs(pending.begin(), pending.end());

    for (uint32_t id : uniqueIDs)
    {
        auto it = m_Shaders.find(id);

        if (it != m_Shaders.end())
            it->second->Reload();
    }
}

TextureHandle ResourceManager::CreateTexture(std::string_view path)
{
    std::string normalizedPath = std::filesystem::path(path).lexically_normal().string();

    if (auto it = m_TexturePathToHandle.find(normalizedPath); it != m_TexturePathToHandle.end())
        return it->second;

    TextureHandle handle(m_NextTextureID++);
    m_Textures.emplace(handle.GetID(), Texture::Create(path));
    m_TexturePathToHandle.emplace(normalizedPath, handle);

    return handle;
}

TextureHandle ResourceManager::CreateTexture(uint32_t width, uint32_t height)
{
    TextureHandle handle(m_NextTextureID++);
    m_Textures.emplace(handle.GetID(), Texture::Create(width, height));

    return handle;
}

void ResourceManager::DestroyTexture(TextureHandle handle)
{
    m_Textures.erase(handle.GetID());

    std::erase_if(m_TexturePathToHandle, [handle](const auto& entry) { return entry.second == handle; });
}

MeshHandle ResourceManager::CreateMesh(std::span<const float> vertices,
                                       std::initializer_list<ShaderDataType> layout,
                                       std::span<const uint32_t> indices)
{
    auto mesh = std::make_unique<Mesh>();

    mesh->vertexBuffer = VertexBuffer::Create(vertices.data(), static_cast<uint32_t>(vertices.size_bytes()));
    mesh->vertexBuffer->SetLayout(std::make_shared<BufferLayout>(layout));

    mesh->indexBuffer = IndexBuffer::Create(indices.data(), static_cast<uint32_t>(indices.size()));

    mesh->vertexArray = VertexArray::Create();
    mesh->vertexArray->AddVertexBuffer(mesh->vertexBuffer);
    mesh->vertexArray->SetIndexBuffer(mesh->indexBuffer);

    MeshHandle handle(m_NextMeshID++);
    m_Meshes.emplace(handle.GetID(), std::move(mesh));

    return handle;
}

void ResourceManager::DestroyMesh(MeshHandle handle)
{
    m_Meshes.erase(handle.GetID());
}

Shader* ResourceManager::GetShader(ShaderHandle handle)
{
    auto it = m_Shaders.find(handle.GetID());

    return it != m_Shaders.end() ? it->second.get() : nullptr;
}

Texture* ResourceManager::GetTexture(TextureHandle handle)
{
    auto it = m_Textures.find(handle.GetID());

    return it != m_Textures.end() ? it->second.get() : nullptr;
}

Mesh* ResourceManager::GetMesh(MeshHandle handle)
{
    auto it = m_Meshes.find(handle.GetID());

    return it != m_Meshes.end() ? it->second.get() : nullptr;
}
}  // namespace Matcha
