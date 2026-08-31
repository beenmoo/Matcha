#include "GLFrameBuffer.h"
#include "Core/Assert.h"

namespace Matcha
{
namespace
{
GLenum ToGLInternalFormat(FrameBufferTextureFormat format)
{
    switch (format)
    {
    case FrameBufferTextureFormat::RGB8:
        return GL_RGB8;
    case FrameBufferTextureFormat::Depth24Stencil8:
        return GL_DEPTH24_STENCIL8;
    case FrameBufferTextureFormat::None:
        break;
    }

    MT_ASSERT(false, "Unknown FrameBufferTextureFormat!");
    return GL_NONE;
}
}  // namespace

GLFrameBuffer::GLFrameBuffer(const FrameBufferSpecification& spec)
    : m_Specification(spec)
{
    Invalidate();
}

void GLFrameBuffer::DestroyGLResources()
{
    glDeleteFramebuffers(1, &m_Handle);
    glDeleteRenderbuffers(1, &m_DepthAttachmentID);
    glDeleteTextures(1, &m_ColorAttachmentID);
}

GLFrameBuffer::~GLFrameBuffer()
{
    DestroyGLResources();
}

void GLFrameBuffer::Bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_Handle);
}

void GLFrameBuffer::Unbind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GLFrameBuffer::Invalidate()
{
    if (m_Handle)
    {
        DestroyGLResources();

        m_Handle = 0;
        m_DepthAttachmentID = 0;
    }

    glCreateFramebuffers(1, &m_Handle);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachmentID);
    glTextureStorage2D(m_ColorAttachmentID, 1, ToGLInternalFormat(m_Specification.textureFormat), m_Specification.width, m_Specification.height);
    glTextureParameteri(m_ColorAttachmentID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_ColorAttachmentID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNamedFramebufferTexture(m_Handle, GL_COLOR_ATTACHMENT0, m_ColorAttachmentID, 0);

    glCreateRenderbuffers(1, &m_DepthAttachmentID);
    glNamedRenderbufferStorage(m_DepthAttachmentID, ToGLInternalFormat(m_Specification.depthFormat), m_Specification.width, m_Specification.height);
    glNamedFramebufferRenderbuffer(m_Handle, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachmentID);

    MT_ASSERT(glCheckNamedFramebufferStatus(m_Handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
}

uint32_t GLFrameBuffer::GetHandle() const
{
    return m_Handle;
}

uint32_t GLFrameBuffer::GetColorAttachmentID() const
{
    return m_ColorAttachmentID;
}

uint32_t GLFrameBuffer::GetDepthAttachmentID() const
{
    return m_DepthAttachmentID;
}

const FrameBufferSpecification& GLFrameBuffer::GetSpecification() const
{
    return m_Specification;
}
}  // namespace Matcha
