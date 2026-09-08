#include "ResourceManager.h"
#include "Core/Logger.h"
#include "IndexBuffer.h"
#include "Primitives.h"
#include "RendererAPI.h"
#include "VertexArray.h"
#include "VertexBuffer.h"

#include <algorithm>
#include <filesystem>
#include <limits>
#include <optional>
#include <unordered_set>

namespace Matcha
{
namespace
{
// Every mesh source in this codebase (CubePrimitive/TrianglePrimitive, ModelLoader's
// CreateMeshFromAiMesh) puts position first in its layout as a Float3 - this finds that
// attribute's byte offset (in `layout`, computed by BufferLayout itself) so bounds can be
// computed generically from whatever layout a caller passes, rather than assuming element 0.
std::optional<size_t> FindPositionByteOffset(const BufferLayout& layout)
{
    for (const BufferLayout::BufferElement& element : layout.GetElements())
        if (element.type == ShaderDataType::Float3)
            return element.offset;

    return std::nullopt;
}

// Scans `vertices` (raw floats, `stride` bytes apart) at `positionByteOffset` for the min/max of
// each of the position attribute's 3 components.
void ComputeLocalBounds(std::span<const float> vertices, const BufferLayout& layout, Vector3& outMin, Vector3& outMax)
{
    std::optional<size_t> positionByteOffset = FindPositionByteOffset(layout);
    if (!positionByteOffset || layout.GetStride() == 0)
        return;

    size_t positionOffset = *positionByteOffset / sizeof(float);
    size_t stride = layout.GetStride() / sizeof(float);

    Vector3 min(std::numeric_limits<float>::max());
    Vector3 max(std::numeric_limits<float>::lowest());

    for (size_t i = positionOffset; i + 2 < vertices.size(); i += stride)
    {
        Vector3 position(vertices[i], vertices[i + 1], vertices[i + 2]);

        min.x = std::min(min.x, position.x);
        min.y = std::min(min.y, position.y);
        min.z = std::min(min.z, position.z);
        max.x = std::max(max.x, position.x);
        max.y = std::max(max.y, position.y);
        max.z = std::max(max.z, position.z);
    }

    outMin = min;
    outMax = max;
}
}  // namespace

ResourceManager::ResourceManager(RendererAPI& rendererAPI)
    : m_RendererAPI(rendererAPI)
{
}

ShaderHandle ResourceManager::CreateShader(std::string_view name, const std::initializer_list<std::string>& paths)
{
    return CreateShaderFromPaths(name, paths);
}

ShaderHandle ResourceManager::CreateShader(std::string_view name, const std::vector<std::string>& paths)
{
    return CreateShaderFromPaths(name, paths);
}

ShaderHandle ResourceManager::CreateShaderFromPaths(std::string_view name, std::span<const std::string> paths)
{
    ShaderHandle handle(m_NextShaderID++);
    m_Shaders.emplace(handle.GetID(), m_RendererAPI.CreateShader(name, paths));

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
    m_Textures.emplace(handle.GetID(), m_RendererAPI.CreateTexture(path));
    m_TexturePathToHandle.emplace(normalizedPath, handle);

    return handle;
}

TextureHandle ResourceManager::CreateTexture(uint32_t width, uint32_t height)
{
    TextureHandle handle(m_NextTextureID++);
    m_Textures.emplace(handle.GetID(), m_RendererAPI.CreateTexture(width, height));

    return handle;
}

void ResourceManager::DestroyTexture(TextureHandle handle)
{
    m_Textures.erase(handle.GetID());

    std::erase_if(m_TexturePathToHandle, [handle](const auto& entry) { return entry.second == handle; });
}

MeshHandle ResourceManager::CreateMesh(std::span<const float> vertices,
                                       std::initializer_list<ShaderDataType> layout,
                                       std::span<const uint32_t> indices,
                                       std::string_view primitiveKind)
{
    auto mesh = std::make_unique<Mesh>();

    auto bufferLayout = std::make_shared<BufferLayout>(layout);
    ComputeLocalBounds(vertices, *bufferLayout, mesh->localBoundsMin, mesh->localBoundsMax);

    mesh->vertexBuffer = m_RendererAPI.CreateVertexBuffer(vertices.data(), static_cast<uint32_t>(vertices.size_bytes()));
    mesh->vertexBuffer->SetLayout(bufferLayout);

    mesh->indexBuffer = m_RendererAPI.CreateIndexBuffer(indices.data(), static_cast<uint32_t>(indices.size()));

    mesh->vertexArray = m_RendererAPI.CreateVertexArray();
    mesh->vertexArray->AddVertexBuffer(mesh->vertexBuffer);
    mesh->vertexArray->SetIndexBuffer(mesh->indexBuffer);

    MeshHandle handle(m_NextMeshID++);
    m_Meshes.emplace(handle.GetID(), std::move(mesh));

    if (!primitiveKind.empty())
        m_MeshPrimitiveKinds.emplace(handle.GetID(), primitiveKind);

    return handle;
}

void ResourceManager::DestroyMesh(MeshHandle handle)
{
    m_Meshes.erase(handle.GetID());
    m_MeshPrimitiveKinds.erase(handle.GetID());

    std::erase_if(m_PrimitiveMeshes, [handle](const auto& entry) { return entry.second == handle; });
}

MeshHandle ResourceManager::GetOrCreatePrimitiveMesh(std::string_view kind)
{
    std::string key(kind);

    if (auto it = m_PrimitiveMeshes.find(key); it != m_PrimitiveMeshes.end())
        return it->second;

    MeshHandle handle;

    if (key == "Cube")
    {
        CubePrimitive cube;
        handle = CreateMesh(cube.vertices, {ShaderDataType::Float3, ShaderDataType::Float3, ShaderDataType::Float2}, cube.indices, "Cube");
    }
    else
    {
        // Add a branch here alongside whatever new primitive gets introduced - SceneSerializer
        // restores a MeshComponent purely by this name, so an unknown one can't be reconstructed.
        MT_CORE_WARN("ResourceManager: unknown mesh primitive kind \"{}\" - no mesh created.", key);
        return handle;
    }

    m_PrimitiveMeshes.emplace(std::move(key), handle);

    return handle;
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

std::string ResourceManager::GetMeshPrimitiveKind(MeshHandle handle) const
{
    auto it = m_MeshPrimitiveKinds.find(handle.GetID());

    return it != m_MeshPrimitiveKinds.end() ? it->second : std::string();
}
}  // namespace Matcha
