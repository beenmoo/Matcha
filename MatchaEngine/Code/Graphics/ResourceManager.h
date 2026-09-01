#pragma once

#include "Graphics/ShaderDataType.h"
#include "Mesh.h"
#include "RenderHandles.h"
#include "Shader.h"
#include "ShaderHotReloader.h"
#include "Texture.h"

#include <cstdint>
#include <initializer_list>
#include <memory>
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
    ResourceManager() = default;

    [[nodiscard]] ShaderHandle CreateShader(std::string_view name, const std::initializer_list<std::string>& paths);

    // For a runtime-sized path list (e.g. SceneSerializer deserializing Shader::GetPaths() back
    // into a real handle) - the initializer_list overload above can't accept that, only a
    // compile-time {"a", "b"} literal. Both just forward into the same std::span-based
    // implementation further down the chain (Shader::Create, RendererAPI::CreateShader, ...).
    [[nodiscard]] ShaderHandle CreateShader(std::string_view name, const std::vector<std::string>& paths);

    void DestroyShader(ShaderHandle handle);

    // Reloads any shaders whose source files changed on disk since the last call.
    // Intended to be called once per frame.
    void ReloadModifiedShaders();

    [[nodiscard]] TextureHandle CreateTexture(std::string_view path);
    [[nodiscard]] TextureHandle CreateTexture(uint32_t width, uint32_t height);
    void DestroyTexture(TextureHandle handle);

    // primitiveKind is empty for the common case (an imported model's mesh, with no way to
    // regenerate it from scratch yet - see SceneSerializer.h) - pass a name (e.g. "Cube") only
    // when the caller can rebuild the exact same geometry procedurally given just that name, so
    // SceneSerializer can round-trip a MeshComponent by regenerating it instead of needing to
    // save geometry data or resolve a source file.
    [[nodiscard]] MeshHandle CreateMesh(std::span<const float> vertices,
                                        std::initializer_list<ShaderDataType> layout,
                                        std::span<const uint32_t> indices,
                                        std::string_view primitiveKind = {});
    void DestroyMesh(MeshHandle handle);

    [[nodiscard]] Shader* GetShader(ShaderHandle handle);
    [[nodiscard]] Texture* GetTexture(TextureHandle handle);
    [[nodiscard]] Mesh* GetMesh(MeshHandle handle);

    // Empty if this handle wasn't created with a primitiveKind (imported meshes, or an invalid
    // handle) - see CreateMesh's own comment.
    [[nodiscard]] std::string GetMeshPrimitiveKind(MeshHandle handle) const;

private:
    uint32_t m_NextShaderID = 1;
    uint32_t m_NextTextureID = 1;
    uint32_t m_NextMeshID = 1;

    std::unordered_map<uint32_t, std::unique_ptr<Shader>> m_Shaders;
    std::unordered_map<uint32_t, std::unique_ptr<Texture>> m_Textures;
    std::unordered_map<uint32_t, std::unique_ptr<Mesh>> m_Meshes;

    // CreateTexture(path) dedupes by normalized path: a texture used by many meshes (materials
    // sharing the same source image, as glTF imports commonly do) is decoded and uploaded once,
    // not once per caller. Only covers the path-based overload - CreateTexture(width, height)
    // (procedural textures) has no natural key to dedupe on.
    std::unordered_map<std::string, TextureHandle> m_TexturePathToHandle;

    // Only ever holds entries for handles created with a non-empty primitiveKind - see
    // CreateMesh's comment.
    std::unordered_map<uint32_t, std::string> m_MeshPrimitiveKinds;

    ShaderHotReloader m_ShaderHotReloader;
};
}  // namespace Matcha
