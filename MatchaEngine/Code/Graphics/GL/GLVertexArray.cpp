#include "GLVertexArray.h"
#include "Core/Assert.h"
#include "GLShaderUtils.h"
#include "Graphics/ShaderDataType.h"

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

void GLVertexArray::BindAttribute(GLuint attribIndex, GLuint bufferHandle, GLsizei stride, GLint componentCount, ShaderDataType type,
                                  bool normalized, GLuint offset)
{
    glVertexArrayVertexBuffer(m_Handle, attribIndex, bufferHandle, 0, stride);
    glVertexArrayAttribFormat(m_Handle, attribIndex, componentCount, ShaderDataTypeToGLDataType(type), normalized, offset);
    glVertexArrayAttribBinding(m_Handle, attribIndex, attribIndex);
    glEnableVertexArrayAttrib(m_Handle, attribIndex);
}

void GLVertexArray::InitAttributes(const GLuint vbIndex)
{
    MT_ASSERT(vbIndex < m_VertexBuffers.size(), "Index out of range!");

    const auto& buffer = m_VertexBuffers[vbIndex];

    GLuint attribIndex = 0;

    auto layout = buffer->GetLayout();

    for (const auto& layoutElement : layout->GetElements())
    {
        if (layoutElement.type == ShaderDataType::Mat3 ||
            layoutElement.type == ShaderDataType::Mat4)
        {
            GLuint count = layoutElement.GetComponentCount();

            for (GLuint index = 0; index < count; ++index)
            {
                BindAttribute(attribIndex, buffer->GetHandle(), layout->GetStride(), count, layoutElement.type, layoutElement.normalized,
                              static_cast<GLuint>(layoutElement.offset + sizeof(GLfloat) * count * index));

                ++attribIndex;
            }
        }
        else
        {
            BindAttribute(attribIndex, buffer->GetHandle(), layout->GetStride(), layoutElement.GetComponentCount(), layoutElement.type,
                          layoutElement.normalized, static_cast<GLuint>(layoutElement.offset));

            ++attribIndex;
        }
    }
}

void GLVertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer> buffer)
{
    MT_ASSERT(buffer->GetLayout(), "Vertex Buffer has no layout!");

    m_VertexBuffers.emplace_back(buffer);

    InitAttributes(static_cast<GLuint>(m_VertexBuffers.size() - 1));
}

void GLVertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer> buffer)
{
    m_IndexBuffer = buffer;

    glVertexArrayElementBuffer(m_Handle, buffer->GetHandle());
}
}  // namespace Matcha