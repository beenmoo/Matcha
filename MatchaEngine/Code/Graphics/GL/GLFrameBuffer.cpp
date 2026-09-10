#include "GLFrameBuffer.h"
#include "Core/Assert.h"

#include <format>

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

    // Drain any error left pending by earlier code, so the checks below only report what this
    // function itself caused.
    while (glGetError() != GL_NO_ERROR)
    {
    }

    glCreateFramebuffers(1, &m_Handle);

    GLenum createError = glGetError();

    glCreateTextures(GL_TEXTURE_2D, 1, &m_ColorAttachmentID);
    glTextureStorage2D(m_ColorAttachmentID, 1, ToGLInternalFormat(m_Specification.textureFormat), m_Specification.width, m_Specification.height);
    glTextureParameteri(m_ColorAttachmentID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(m_ColorAttachmentID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glNamedFramebufferTexture(m_Handle, GL_COLOR_ATTACHMENT0, m_ColorAttachmentID, 0);

    glCreateRenderbuffers(1, &m_DepthAttachmentID);
    glNamedRenderbufferStorage(m_DepthAttachmentID, ToGLInternalFormat(m_Specification.depthFormat), m_Specification.width, m_Specification.height);
    glNamedFramebufferRenderbuffer(m_Handle, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_DepthAttachmentID);

    // Dimensions included since this can otherwise fire during MatchaEditor's viewport resize (a
    // degenerate size mid-drag, or these GL calls landing in the wrong context if something else
    // stole "current" on this thread) - easy to misdiagnose blind.
    GLenum status = glCheckNamedFramebufferStatus(m_Handle, GL_FRAMEBUFFER);
    MT_ASSERT(status == GL_FRAMEBUFFER_COMPLETE,
              std::format("Framebuffer is incomplete! status=0x{:X} width={} height={} handle={} color={} depth={} "
                          "createError=0x{:X} lastError=0x{:X} (status 0 means the GL call itself errored - usually no "
                          "current context, or these names belong to a different context)",
                          status, m_Specification.width, m_Specification.height, m_Handle, m_ColorAttachmentID,
                          m_DepthAttachmentID, createError, glGetError()));
}

void GLFrameBuffer::Resize(uint32_t width, uint32_t height)
{
    m_Specification.width = width;
    m_Specification.height = height;
    Invalidate();
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
