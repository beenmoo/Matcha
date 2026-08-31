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

    ShaderHotReloader m_ShaderHotReloader;
};
}  // namespace Matcha
