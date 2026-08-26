#include "GLIndexBuffer.h"

namespace Matcha
{
GLIndexBuffer::GLIndexBuffer(const GLuint* indices, GLuint count)
{
    glCreateBuffers(1, &m_Handle);
    glNamedBufferData(m_Handle, count * sizeof(GLuint), indices, GL_STATIC_DRAW);

    for (size_t i = 0; i < count; ++i)
        m_Indices.emplace_back(indices[i]);
}

GLIndexBuffer::~GLIndexBuffer()
{
    glDeleteBuffers(1, &m_Handle);
}

void GLIndexBuffer::AddIndices(std::initializer_list<GLuint> indices)
{
    m_Indices.insert(m_Indices.end(), indices.begin(), indices.end());

    glNamedBufferData(m_Handle, m_Indices.size() * sizeof(GLuint), m_Indices.data(), m_DrawType);
}

void GLIndexBuffer::SetIndices(const GLuint* indices, GLuint count)
{
    glNamedBufferSubData(m_Handle, 0, count * sizeof(GLuint), indices);

    Clear();

    for (size_t i = 0; i < count; ++i)
        m_Indices.emplace_back(indices[i]);
}

void GLIndexBuffer::SetIndicesNew(const GLuint* indices, GLuint count)
{
    glNamedBufferData(m_Handle, count * sizeof(GLuint), indices, GL_STATIC_DRAW);

    Clear();

    for (size_t i = 0; i < count; ++i)
        m_Indices.emplace_back(indices[i]);
}

void GLIndexBuffer::Clear()
{
    m_Indices.clear();
}

GLuint GLIndexBuffer::GetHandle() const
{
    return m_Handle;
}

GLuint GLIndexBuffer::GetCount() const
{
    return static_cast<GLuint>(m_Indices.size());
}
}  // namespace Matcha