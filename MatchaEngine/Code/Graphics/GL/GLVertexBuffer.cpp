#include "GLVertexBuffer.h"

namespace Matcha
{
GLVertexBuffer::GLVertexBuffer(GLuint sizeInBytes)
{
    glCreateBuffers(1, &m_Handle);
    glNamedBufferData(m_Handle, sizeInBytes, nullptr, GL_DYNAMIC_DRAW);
}

GLVertexBuffer::GLVertexBuffer(const GLfloat* vertices, GLuint sizeInBytes)
{
    glCreateBuffers(1, &m_Handle);
    glNamedBufferData(m_Handle, sizeInBytes, vertices, GL_STATIC_DRAW);

    for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
        m_Vertices.emplace_back(vertices[i]);
}

GLVertexBuffer::~GLVertexBuffer()
{
    glDeleteBuffers(1, &m_Handle);
}

void GLVertexBuffer::AddVertex(std::initializer_list<GLfloat> vertex)
{
    m_Vertices.insert(m_Vertices.end(), vertex.begin(), vertex.end());

    glNamedBufferData(m_Handle, GetSizeInBytes(), m_Vertices.data(), m_DrawType);
}

void GLVertexBuffer::SetVertices(const GLfloat* vertices, GLuint sizeInBytes)
{
    glNamedBufferSubData(m_Handle, 0, sizeInBytes, vertices);

    Clear();

    for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
        m_Vertices.emplace_back(vertices[i]);
}

void GLVertexBuffer::SetVerticesNew(const GLfloat* vertices, GLuint sizeInBytes)
{
    glNamedBufferData(m_Handle, sizeInBytes, vertices, m_DrawType);

    Clear();

    for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
        m_Vertices.emplace_back(vertices[i]);
}

void GLVertexBuffer::SetDrawType(GLenum drawType)
{
    m_DrawType = drawType;
}

uint32_t GLVertexBuffer::GetSizeInBytes() const
{
    return static_cast<GLuint>(m_Vertices.size() * sizeof(GLfloat));
}

void GLVertexBuffer::Clear()
{
    m_Vertices.clear();
}

void GLVertexBuffer::SetLayout(const std::shared_ptr<BufferLayout> layout)
{
    m_Layout = layout;
}

const GLVertexBuffer::BufferLayout* GLVertexBuffer::GetLayout() const
{
    return m_Layout.get();
}

GLuint GLVertexBuffer::GetHandle() const
{
    return m_Handle;
}
}  // namespace Matcha