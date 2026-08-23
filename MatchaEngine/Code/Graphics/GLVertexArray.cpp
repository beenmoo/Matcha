#include "GLVertexArray.h"
#include "Core/Assert.h"
#include "GLVertexBuffer.h"
#include "GLIndexBuffer.h"

namespace Matcha
{
GLVertexArray::GLVertexArray()
{
    glCreateVertexArrays(1, &mHandle);
}

GLVertexArray::~GLVertexArray()
{
    glDeleteVertexArrays(1, &mHandle);
}

void GLVertexArray::Bind() const
{
    glBindVertexArray(mHandle);
}

void GLVertexArray::Unbind() const
{
    glBindVertexArray(0);
}

void GLVertexArray::InitAttributes(const GLuint vbIndex)
{
    MT_ASSERT(vbIndex < mVertexBuffers.size(), "Index out of range!");

    const auto& buffer = mVertexBuffers[vbIndex];

    GLuint attribIndex = 0;

    auto layout = buffer->GetLayout();

    for (const auto& e : layout->GetElements())
    {
        if (e.type == ShaderUtils::ShaderDataType::Mat3 ||
            e.type == ShaderUtils::ShaderDataType::Mat4)
        {
            GLuint count = e.GetComponentCount();

            for (GLuint i = 0; i < count; i++)
            {
                glVertexArrayVertexBuffer(mHandle, attribIndex, buffer->GetHandle(), 0, layout->GetStride());
                glVertexArrayAttribFormat(mHandle,
                                          attribIndex,
                                          count,
                                          ShaderUtils::ShaderDataTypeToGLDataType(e.type),
                                          e.normalized,
                                          static_cast<GLuint>(e.offset + sizeof(GLfloat) * count * i));
                glVertexArrayAttribBinding(mHandle, attribIndex, attribIndex);
                glVertexArrayBindingDivisor(mHandle, attribIndex, 1);
                glEnableVertexArrayAttrib(mHandle, attribIndex++);
            }
        }
        else
        {
            glVertexArrayVertexBuffer(mHandle, attribIndex, buffer->GetHandle(), 0, layout->GetStride());
            glVertexArrayAttribFormat(mHandle,
                                      attribIndex,
                                      e.GetComponentCount(),
                                      ShaderUtils::ShaderDataTypeToGLDataType(e.type),
                                      e.normalized,
                                      static_cast<GLuint>(e.offset));
            glVertexArrayAttribBinding(mHandle, attribIndex, attribIndex);
            glEnableVertexArrayAttrib(mHandle, attribIndex++);
        }
    }
}

void GLVertexArray::AddVertexBuffer(const std::shared_ptr<GLVertexBuffer> buffer)
{
    MT_ASSERT(buffer->GetLayout(), "Vertex Buffer has no layout!");

    mVertexBuffers.emplace_back(buffer);

    InitAttributes(static_cast<GLuint>(mVertexBuffers.size() - 1));
}

void GLVertexArray::SetIndexBuffer(const std::shared_ptr<GLIndexBuffer> buffer)
{
    mIndexBuffer = buffer;

    glVertexArrayElementBuffer(mHandle, buffer->GetHandle());
}
}  // namespace Matcha