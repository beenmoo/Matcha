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

    [[nodiscard]] std::unique_ptr<Texture> CreateTexture(uint32_t width, uint32_t height) override;
    [[nodiscard]] std::unique_ptr<Texture> CreateTexture(std::string_view path) override;
    [[nodiscard]] std::unique_ptr<Shader> CreateShader(std::string_view name, const std::initializer_list<std::string>& paths) override;
    [[nodiscard]] std::unique_ptr<VertexArray> CreateVertexArray() override;
    [[nodiscard]] std::shared_ptr<IndexBuffer> CreateIndexBuffer(const uint32_t* indices, uint32_t count) override;
    [[nodiscard]] std::shared_ptr<VertexBuffer> CreateVertexBuffer(const float* vertices, uint32_t sizeInBytes) override;
    [[nodiscard]] std::unique_ptr<UniformBuffer> CreateUniformBuffer(uint32_t size, uint32_t binding) override;
    [[nodiscard]] std::unique_ptr<FrameBuffer> CreateFrameBuffer(const FrameBufferSpecification& spec) override;
};
}  // namespace Matcha
