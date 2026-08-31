#pragma once

#include "Graphics/FrameBuffer.h"

#include <glad/glad.h>

namespace Matcha
{
class GLFrameBuffer final : public FrameBuffer
{
public:
    explicit GLFrameBuffer(const FrameBufferSpecification& spec);
    ~GLFrameBuffer() override;

    GLFrameBuffer(const GLFrameBuffer&) = delete;
    GLFrameBuffer& operator=(const GLFrameBuffer&) = delete;

    void Bind() const override;
    void Unbind() const override;

    void Invalidate() override;

    [[nodiscard]] uint32_t GetHandle() const override;
    [[nodiscard]] uint32_t GetColorAttachmentID() const override;
    [[nodiscard]] uint32_t GetDepthAttachmentID() const override;

    [[nodiscard]] const FrameBufferSpecification& GetSpecification() const override;

private:
    void DestroyGLResources();

private:
    GLuint m_Handle = 0;
    GLuint m_ColorAttachmentID = 0;
    GLuint m_DepthAttachmentID = 0;
    FrameBufferSpecification m_Specification;
};
}  // namespace Matcha
