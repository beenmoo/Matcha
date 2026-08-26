#include "GLVertexArray.h"
#include "Core/Assert.h"
#include "GLVertexBuffer.h"
#include "GLIndexBuffer.h"

namespace Matcha
{
GLVertexArray::GLVertexArray()
{
    glCreateVertexArrays(1, &m_Handle);
}

GLVertexArray::~GLVertexArray()
{
    glDeleteVertexArrays(1, &m_Handle);
}

void GLVertexArray::Bind() const
{
    glBindVertexArray(m_Handle);
}

void GLVertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void GLVertexArray::InitAttributes(const GLuint vbIndex)
{
    MT_ASSERT(vbIndex < m_VertexBuffers.size(), "Index out of range!");

    const auto& buffer = m_VertexBuffers[vbIndex];

    GLuint attribIndex = 0;

    auto layout = buffer->GetLayout();

    for (const auto& e : layout->GetElements())
    {
        if (e.type == Utils::ShaderDataType::Mat3 ||
            e.type == Utils::ShaderDataType::Mat4)
        {
            GLuint count = e.GetComponentCount();

            for (GLuint i = 0; i < count; i++)
            {
                glVertexArrayVertexBuffer(m_Handle, attribIndex, buffer->GetHandle(), 0, layout->GetStride());
                glVertexArrayAttribFormat(m_Handle,
                                          attribIndex,
                                          count,
                                          Utils::ShaderDataTypeToGLDataType(e.type),
                                          e.normalized,
                                          static_cast<GLuint>(e.offset + sizeof(GLfloat) * count * i));
                glVertexArrayAttribBinding(m_Handle, attribIndex, attribIndex);
                glVertexArrayBindingDivisor(m_Handle, attribIndex, 1);
                glEnableVertexArrayAttrib(m_Handle, attribIndex++);
            }
        }
        else
        {
            glVertexArrayVertexBuffer(m_Handle, attribIndex, buffer->GetHandle(), 0, layout->GetStride());
            glVertexArrayAttribFormat(m_Handle,
                                      attribIndex,
                                      e.GetComponentCount(),
                                      Utils::ShaderDataTypeToGLDataType(e.type),
                                      e.normalized,
                                      static_cast<GLuint>(e.offset));
            glVertexArrayAttribBinding(m_Handle, attribIndex, attribIndex);
            glEnableVertexArrayAttrib(m_Handle, attribIndex++);
        }
    }
}

void GLVertexArray::AddVertexBuffer(const std::shared_ptr<GLVertexBuffer> buffer)
{
    MT_ASSERT(buffer->GetLayout(), "Vertex Buffer has no layout!");

    m_VertexBuffers.emplace_back(buffer);

    InitAttributes(static_cast<GLuint>(m_VertexBuffers.size() - 1));
}

void GLVertexArray::SetIndexBuffer(const std::shared_ptr<GLIndexBuffer> buffer)
{
    m_IndexBuffer = buffer;

    glVertexArrayElementBuffer(m_Handle, buffer->GetHandle());
}
}  // namespace Matcha