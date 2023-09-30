#pragma once

namespace Matcha
{
    class FrameBuffer
    {
    public:
        struct FrameBufferSpecification
        {
            uint32_t mWidth = 0, mHeight = 0;
            uint32_t mNumSamples = 1;
            GLenum mTextureFormat = GL_RGB;
            GLenum mDepthFormat = GL_DEPTH24_STENCIL8;
        };

    public:
        FrameBuffer(const FrameBufferSpecification& spec);
        ~FrameBuffer();

        void Bind() const;
        void Unbind() const;

        void Invalidate();

        uint32_t GetObjectID() const;
        uint32_t GetColorAttachmentID() const;
        uint32_t GetDepthAttachmentID() const;
     
        const FrameBufferSpecification& GetSpecification() const;

    private:
        uint32_t mObjectID = 0;
        uint32_t mColorAttachmentID = 0;
        uint32_t mDepthAttachmentID = 0;
        FrameBufferSpecification mSpecification;
    };
}