#include "GLUniformBuffer.h"

#include <glad/glad.h>

namespace Matcha
{
GLUniformBuffer::GLUniformBuffer(uint32_t size, uint32_t binding)
{
    glCreateBuffers(1, &m_Handle);
    glNamedBufferData(m_Handle, size, nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_Handle);
}

GLUniformBuffer::~GLUniformBuffer()
{
    glDeleteBuffers(1, &m_Handle);
}

void GLUniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
{
    glNamedBufferSubData(m_Handle, offset, size, data);
}
}  // namespace Matcha
