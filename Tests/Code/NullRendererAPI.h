#pragma once

#include "Graphics/RendererAPI.h"

namespace MatchaTests
{
// A RendererAPI that creates nothing and draws nothing, for tests that need a ResourceManager but
// have no GL context to back one (this binary never opens a window). Every factory returns null:
// the tests using it exercise code paths that don't touch the returned resources - notably
// SceneSerializer, which round-trips component *data* and deliberately avoids
// ResourceManager::CreateMesh for exactly this reason.
//
// This is the seam that ResourceManager taking a RendererAPI& (rather than reaching for a global
// "currently active" one) buys: a test can hand it a double, with no process-wide state to set up
// or tear down between tests.
class NullRendererAPI final : public Matcha::RendererAPI
{
public:
    void Init() override {}
    void SetViewport(uint32_t, uint32_t, uint32_t, uint32_t) override {}
    void SetClearColor(const Matcha::Vector4&) override {}
    void Clear() override {}
    void DrawIndexed(const Matcha::VertexArray&, uint32_t) override {}

    std::unique_ptr<Matcha::Texture> CreateTexture(uint32_t, uint32_t) override { return nullptr; }
    std::unique_ptr<Matcha::Texture> CreateTexture(std::string_view) override { return nullptr; }
    std::unique_ptr<Matcha::Shader> CreateShader(std::string_view, std::span<const std::string>) override { return nullptr; }
    std::unique_ptr<Matcha::VertexArray> CreateVertexArray() override { return nullptr; }
    std::shared_ptr<Matcha::IndexBuffer> CreateIndexBuffer(const uint32_t*, uint32_t) override { return nullptr; }
    std::shared_ptr<Matcha::VertexBuffer> CreateVertexBuffer(const float*, uint32_t) override { return nullptr; }
    std::unique_ptr<Matcha::UniformBuffer> CreateUniformBuffer(uint32_t, uint32_t) override { return nullptr; }
    std::unique_ptr<Matcha::FrameBuffer> CreateFrameBuffer(const Matcha::FrameBufferSpecification&) override { return nullptr; }
};
}  // namespace MatchaTests
