#pragma once

#include "RenderHandles.h"
#include "Utils/ShaderDataType.h"

#include <cstdint>
#include <initializer_list>
#include <span>
#include <string>
#include <string_view>

namespace Matcha
{
class ResourceManager
{
public:
    virtual ~ResourceManager() = default;

    [[nodiscard]] virtual ShaderHandle CreateShader(std::string_view name, const std::initializer_list<std::string>& paths) = 0;
    virtual void DestroyShader(ShaderHandle handle) = 0;

    // Reloads any shaders whose source files changed on disk since the last call.
    // Intended to be called once per frame.
    virtual void ReloadModifiedShaders() = 0;

    [[nodiscard]] virtual TextureHandle CreateTexture(std::string_view path) = 0;
    [[nodiscard]] virtual TextureHandle CreateTexture(uint32_t width, uint32_t height) = 0;
    virtual void DestroyTexture(TextureHandle handle) = 0;

    [[nodiscard]] virtual MeshHandle CreateMesh(std::span<const float> vertices,
                                                std::initializer_list<Utils::ShaderDataType> layout,
                                                std::span<const uint32_t> indices) = 0;
    virtual void DestroyMesh(MeshHandle handle) = 0;
};
}  // namespace Matcha
