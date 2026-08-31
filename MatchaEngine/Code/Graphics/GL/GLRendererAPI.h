#pragma once

#include "Graphics/RendererAPI.h"

namespace Matcha
{
class GLRendererAPI final : public RendererAPI
{
public:
    void Init() override;
    void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;
    void SetClearColor(const Vector4& color) override;
    void Clear() override;

    void DrawIndexed(const VertexArray& vertexArray, uint32_t indexCount = 0) override;
};
}  // namespace Matcha
