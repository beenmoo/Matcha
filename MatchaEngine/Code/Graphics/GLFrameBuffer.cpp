#include "GLFrameBuffer.h"
#include "Core/Assert.h"

namespace Matcha
{
GLFrameBuffer::GLFrameBuffer(const FrameBufferSpecification& spec) : mSpecification(spec)
{
    Invalidate();
}

GLFrameBuffer::~GLFrameBuffer()
{
    glDeleteFramebuffers(1, &mHandle);
    glDeleteRenderbuffers(1, &mDepthAttachmentID);
    glDeleteTextures(1, &mColorAttachmentID);
}

void GLFrameBuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, mHandle);
}

void GLFrameBuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLFrameBuffer::Invalidate()
{
    if (mHandle)
    {
        glDeleteFramebuffers(1, &mHandle);
        glDeleteRenderbuffers(1, &mDepthAttachmentID);
        glDeleteTextures(1, &mColorAttachmentID);

        mHandle = 0;
        mDepthAttachmentID = 0;
    }

    glCreateFramebuffers(1, &mHandle);

    glCreateTextures(GL_TEXTURE_2D, 1, &mColorAttachmentID);
    glTextureStorage2D(mColorAttachmentID, 1, mSpecification.mTextureFormat, mSpecification.mWidth, mSpecification.mHeight);
    glTextureParameteri(mColorAttachmentID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(mColorAttachmentID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNamedFramebufferTexture(mHandle, GL_COLOR_ATTACHMENT0, mColorAttachmentID, 0);

    glCreateRenderbuffers(1, &mDepthAttachmentID);
    glNamedRenderbufferStorage(mDepthAttachmentID, mSpecification.mDepthFormat, mSpecification.mWidth, mSpecification.mHeight);
    glNamedFramebufferRenderbuffer(mHandle, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDepthAttachmentID);

    MT_ASSERT(glCheckNamedFramebufferStatus(mHandle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
}

GLuint GLFrameBuffer::GetHandle() const
{
    return mHandle;
}

GLuint GLFrameBuffer::GetColorAttachmentID() const
{
    return mColorAttachmentID;
}

GLuint GLFrameBuffer::GetDepthAttachmentID() const
{
    return mDepthAttachmentID;
}

const GLFrameBuffer::FrameBufferSpecification& GLFrameBuffer::GetSpecification() const
{
    return mSpecification;
}
}  // namespace Matcha