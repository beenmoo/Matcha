#include "GLVertexBuffer.h"

namespace Matcha
{
GLVertexBuffer::GLVertexBuffer(const GLfloat* vertices, GLuint sizeInBytes)
    : m_SizeInBytes(sizeInBytes)
{
    glCreateBuffers(1, &m_Handle);
    glNamedBufferData(m_Handle, sizeInBytes, vertices, GL_STATIC_DRAW);
}

GLVertexBuffer::~GLVertexBuffer()
{
    glDeleteBuffers(1, &m_Handle);
}

uint32_t GLVertexBuffer::GetSizeInBytes() const
{
    return m_SizeInBytes;
}

void GLVertexBuffer::SetLayout(const std::shared_ptr<BufferLayout> layout)
{
    m_Layout = layout;
}

const BufferLayout* GLVertexBuffer::GetLayout() const
{
    return m_Layout.get();
}

uint32_t GLVertexBuffer::GetHandle() const
{
    return m_Handle;
}
}  // namespace Matcha
