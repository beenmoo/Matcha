#pragma once

#include "FrameBuffer.h"
#include "Math/Vector.h"
#include "Shader.h"
#include "Texture.h"
#include "UniformBuffer.h"
#include "VertexArray.h"

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <string>
#include <string_view>

namespace Matcha
{
class RendererAPI
{
public:
    enum class API
    {
        None = 0,
        OpenGL,
        Vulkan,
        DirectX12
    };

public:
    virtual ~RendererAPI() = default;

    virtual void Init() = 0;
    virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) = 0;
    virtual void SetClearColor(const Vector4& color) = 0;
    virtual void Clear() = 0;

    virtual void DrawIndexed(const VertexArray& vertexArray, uint32_t indexCount = 0) = 0;

    // One factory method per resource type, implemented by each concrete backend (GLRendererAPI,
    // eventually Vulkan/DirectX12 ones) - replaces the old pattern of every resource type (Texture,
    // Shader, ...) switching on the active API enum itself in its own Create(), which duplicated
    // the same switch across 8 files. Adding a backend now means implementing one class, not
    // touching every resource type.
    [[nodiscard]] virtual std::unique_ptr<Texture> CreateTexture(uint32_t width, uint32_t height) = 0;
    [[nodiscard]] virtual std::unique_ptr<Texture> CreateTexture(std::string_view path) = 0;
    [[nodiscard]] virtual std::unique_ptr<Shader> CreateShader(std::string_view name, const std::initializer_list<std::string>& paths) = 0;
    [[nodiscard]] virtual std::unique_ptr<VertexArray> CreateVertexArray() = 0;
    [[nodiscard]] virtual std::shared_ptr<IndexBuffer> CreateIndexBuffer(const uint32_t* indices, uint32_t count) = 0;
    [[nodiscard]] virtual std::shared_ptr<VertexBuffer> CreateVertexBuffer(const float* vertices, uint32_t sizeInBytes) = 0;
    [[nodiscard]] virtual std::unique_ptr<UniformBuffer> CreateUniformBuffer(uint32_t size, uint32_t binding) = 0;
    [[nodiscard]] virtual std::unique_ptr<FrameBuffer> CreateFrameBuffer(const FrameBufferSpecification& spec) = 0;

    [[nodiscard]] static std::unique_ptr<RendererAPI> Create(API api);

    [[nodiscard]] static std::string ToString(API api);
};

// The one RendererAPI instance Application owns (set once, from Application's constructor) -
// resource types' Create() functions dispatch to this instance's virtual CreateXxx() methods
// instead of switching on an API enum themselves.
[[nodiscard]] RendererAPI& GetActiveRendererAPI();
void SetActiveRendererAPI(RendererAPI& api);
}  // namespace Matcha
