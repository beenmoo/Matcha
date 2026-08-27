#include "GLFrameBuffer.h"
#include "Core/Assert.h"

namespace Matcha
{
GLFrameBuffer::GLFrameBuffer(const FrameBufferSpecification& spec)
    : m_Specification(spec)
{
    Invalidate();
}

GLFrameBuffer::~GLFrameBuffer()
{
    glDeleteFramebuffers(1, &m_Handle);
    glDeleteRenderbuffers(1, &m_DepthAttachmentID);
    glDeleteTextures(1, &m_ColorAttachmentID);
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
        glDeleteFramebuffers(1, &m_Handle);
        glDeleteRenderbuffers(1, &m_DepthAttachmentID);
        glDeleteTextures(1, &m_ColorAttachmentID);

        m_Handle = 0;
        m_DepthAttachmentID = 0;
    }

    glCreateFramebuffers(1, &m_Handle);

    glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachmentID);
    glTextureStorage2D(m_ColorAttachmentID, 1, m_Specification.m_TextureFormat, m_Specification.m_Width, m_Specification.m_Height);
    glTextureParameteri(m_ColorAttachmentID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_ColorAttachmentID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNamedFramebufferTexture(m_Handle, GL_COLOR_ATTACHMENT0, m_ColorAttachmentID, 0);

    glCreateRenderbuffers(1, &m_DepthAttachmentID);
    glNamedRenderbufferStorage(m_DepthAttachmentID, m_Specification.m_DepthFormat, m_Specification.m_Width, m_Specification.m_Height);
    glNamedFramebufferRenderbuffer(m_Handle, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachmentID);

    MT_ASSERT(glCheckNamedFramebufferStatus(m_Handle, GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");
}

GLuint GLFrameBuffer::GetHandle() const
{
    return m_Handle;
}

GLuint GLFrameBuffer::GetColorAttachmentID() const
{
    return m_ColorAttachmentID;
}

GLuint GLFrameBuffer::GetDepthAttachmentID() const
{
    return m_DepthAttachmentID;
}

const GLFrameBuffer::FrameBufferSpecification& GLFrameBuffer::GetSpecification() const
{
    return m_Specification;
}
}  // namespace Matcha