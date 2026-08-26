#pragma once

#include <glad/glad.h>

namespace Matcha
{
class GLFrameBuffer
{
public:
    struct FrameBufferSpecification
    {
        GLuint m_Width = 0, m_Height = 0;
        GLuint m_NumSamples = 1;
        GLenum m_TextureFormat = GL_RGB8;
        GLenum m_DepthFormat = GL_DEPTH24_STENCIL8;
    };

public:
    GLFrameBuffer(const FrameBufferSpecification& spec);
    ~GLFrameBuffer();

    void Bind() const;
    void Unbind() const;

    void Invalidate();

    [[nodiscard]] GLuint GetHandle() const;
    [[nodiscard]] GLuint GetColorAttachmentID() const;
    [[nodiscard]] GLuint GetDepthAttachmentID() const;

    [[nodiscard]] const FrameBufferSpecification& GetSpecification() const;

private:
    GLuint m_Handle = 0;
    GLuint m_ColorAttachmentID = 0;
    GLuint m_DepthAttachmentID = 0;
    FrameBufferSpecification m_Specification;
};
}  // namespace Matcha