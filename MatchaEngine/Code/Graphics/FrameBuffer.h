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

    // Updates the specification's width/height and rebuilds every GL resource at the new size
    // (via Invalidate()) - note this also unbinds the framebuffer as a side effect (deleting the
    // currently-bound framebuffer object reverts GL's binding to 0 per spec), so a caller relying
    // on this staying bound across resizes (see Editor) needs to Bind() again afterward.
    virtual void Resize(uint32_t width, uint32_t height) = 0;

    [[nodiscard]] virtual uint32_t GetColorAttachmentID() const = 0;
    [[nodiscard]] virtual uint32_t GetDepthAttachmentID() const = 0;

    [[nodiscard]] virtual const FrameBufferSpecification& GetSpecification() const = 0;

};
}  // namespace Matcha
