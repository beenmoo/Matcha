#pragma once

#include "Math/Vector.h"
#include "VertexArray.h"

#include <cstdint>
#include <memory>

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

    [[nodiscard]] static std::unique_ptr<RendererAPI> Create(API api);

    [[nodiscard]] static std::string ToString(RendererAPI::API api);
};

[[nodiscard]] RendererAPI::API GetRendererAPI();
void SetRendererAPI(RendererAPI::API api);
}  // namespace Matcha
