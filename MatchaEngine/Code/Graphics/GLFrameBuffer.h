#pragma once

#include <glad/glad.h>

namespace Matcha
{
class GLFrameBuffer
{
public:
    struct FrameBufferSpecification
    {
        GLuint mWidth = 0, mHeight = 0;
        GLuint mNumSamples = 1;
        GLenum mTextureFormat = GL_RGB8;
        GLenum mDepthFormat = GL_DEPTH24_STENCIL8;
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
    GLuint mHandle = 0;
    GLuint mColorAttachmentID = 0;
    GLuint mDepthAttachmentID = 0;
    FrameBufferSpecification mSpecification;
};
}  // namespace Matcha