#include "GLFrameBuffer.h"
#include "Core/Assert.h"

namespace Matcha
{
    GLFrameBuffer::GLFrameBuffer(const FrameBufferSpecification& spec) :
        mSpecification(spec)
    {
        Invalidate();
    }

    GLFrameBuffer::~GLFrameBuffer()
    {
        glDeleteFramebuffers(1, &mObjectID);
        glDeleteRenderbuffers(1, &mDepthAttachmentID);
        glDeleteTextures(1, &mColorAttachmentID);
    }

    void GLFrameBuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mObjectID);
    }

    void GLFrameBuffer::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void GLFrameBuffer::Invalidate()
    {
        if (mObjectID)
        {
            glDeleteFramebuffers(1, &mObjectID);
            glDeleteRenderbuffers(1, &mDepthAttachmentID);
            glDeleteTextures(1, &mColorAttachmentID);

            mObjectID = 0;
            mDepthAttachmentID = 0;
        }

        glCreateFramebuffers(1, &mObjectID);
        Bind();

        glCreateTextures(GL_TEXTURE_2D, 1, &mColorAttachmentID);
        glBindTexture(GL_TEXTURE_2D, mColorAttachmentID);
        glTexImage2D(GL_TEXTURE_2D, 0,
                     mSpecification.mTextureFormat,
                     mSpecification.mWidth, mSpecification.mHeight,
                     0,
                     mSpecification.mTextureFormat,
                     GL_UNSIGNED_BYTE,
                     nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorAttachmentID, 0);

        glCreateRenderbuffers(1, &mDepthAttachmentID);
        glBindRenderbuffer(GL_RENDERBUFFER, mDepthAttachmentID);
        glRenderbufferStorage(GL_RENDERBUFFER, 
                              mSpecification.mDepthFormat, 
                              mSpecification.mWidth, mSpecification.mHeight);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        MT_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

        Unbind();
    }

    GLuint GLFrameBuffer::GetObjectID() const
    {
        return mObjectID;
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
}