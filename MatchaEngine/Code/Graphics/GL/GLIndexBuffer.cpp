#include "GLIndexBuffer.h"

namespace Matcha
{
GLIndexBuffer::GLIndexBuffer(const GLuint* indices, GLuint count)
    : m_Count(count)
{
    glCreateBuffers(1, &m_Handle);
    glNamedBufferData(m_Handle, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
}

GLIndexBuffer::~GLIndexBuffer()
{
    glDeleteBuffers(1, &m_Handle);
}

GLuint GLIndexBuffer::GetHandle() const
{
    return m_Handle;
}

GLuint GLIndexBuffer::GetCount() const
{
    return m_Count;
}
}  // namespace Matcha