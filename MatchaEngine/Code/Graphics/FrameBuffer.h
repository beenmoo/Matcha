#pragma once

#include <cstdint>
#include <memory>

namespace Matcha
{
enum class FrameBufferTextureFormat
{
    None = 0,
    RGB8,
    Depth24Stencil8
};

struct FrameBufferSpecification
{
    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t samples = 1;
    FrameBufferTextureFormat textureFormat = FrameBufferTextureFormat::RGB8;
    FrameBufferTextureFormat depthFormat = FrameBufferTextureFormat::Depth24Stencil8;
};

class FrameBuffer
{
public:
    virtual ~FrameBuffer() = default;

    virtual void Bind() const = 0;
    virtual void Unbind() const = 0;

    virtual void Invalidate() = 0;

    [[nodiscard]] virtual uint32_t GetHandle() const = 0;
    [[nodiscard]] virtual uint32_t GetColorAttachmentID() const = 0;
    [[nodiscard]] virtual uint32_t GetDepthAttachmentID() const = 0;

    [[nodiscard]] virtual const FrameBufferSpecification& GetSpecification() const = 0;

    [[nodiscard]] static std::unique_ptr<FrameBuffer> Create(const FrameBufferSpecification& spec);
};
}  // namespace Matcha
