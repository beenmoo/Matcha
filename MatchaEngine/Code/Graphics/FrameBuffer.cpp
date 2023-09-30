#include "FrameBuffer.h"
#include "Core/Assert.h"

namespace Matcha
{
    FrameBuffer::FrameBuffer(const FrameBufferSpecification& spec) :
        mSpecification(spec)
    {
        Invalidate();
    }

    FrameBuffer::~FrameBuffer()
    {
        glDeleteFramebuffers(1, &mObjectID);
        glDeleteRenderbuffers(1, &mDepthAttachmentID);
        glDeleteTextures(1, &mColorAttachmentID);
    }

    void FrameBuffer::Bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, mObjectID);
    }

    void FrameBuffer::Unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void FrameBuffer::Invalidate()
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

    uint32_t FrameBuffer::GetObjectID() const
    {
        return mObjectID;
    }

    uint32_t FrameBuffer::GetColorAttachmentID() const
    {
        return mColorAttachmentID;
    }

    uint32_t FrameBuffer::GetDepthAttachmentID() const
    {
        return mDepthAttachmentID;
    }

    const FrameBuffer::FrameBufferSpecification& FrameBuffer::GetSpecification() const
    {
        return mSpecification;
    }
}