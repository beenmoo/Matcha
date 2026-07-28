#include "GLVertexArray.h"
#include "Core/Assert.h"
#include "GLVertexBuffer.h"
#include "GLIndexBuffer.h"

namespace Matcha
{
    GLVertexArray::GLVertexArray()
    {
        glCreateVertexArrays(1, &mObjectID);
    }
    
    GLVertexArray::~GLVertexArray()
    {
        glDeleteVertexArrays(1, &mObjectID);
    }
    
    void GLVertexArray::Bind() const
    {
        glBindVertexArray(mObjectID);
    }
    
    void GLVertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }
    
    void GLVertexArray::InitAttributes(const GLuint vbIndex)
    {
        MT_ASSERT(vbIndex < mVertexBuffers.size(), "Index out of range!");

        Bind();

        const auto& buffer = mVertexBuffers[vbIndex];
        buffer->Bind();

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
                    glVertexAttribPointer(attribIndex,
                                          count,
                                          ShaderUtils::ShaderDataTypeToGLDataType(e.type),
                                          e.normalized,
                                          layout->GetStride(),
                                          (const GLvoid*)(e.offset + sizeof(GLfloat) * count * i));
                    glVertexAttribDivisor(attribIndex, 1);
                    glEnableVertexAttribArray(attribIndex++);
                }
            }
            else
            {
                glVertexAttribPointer(attribIndex,
                                      e.GetComponentCount(),
                                      ShaderUtils::ShaderDataTypeToGLDataType(e.type),
                                      e.normalized,
                                      layout->GetStride(),
                                      (const GLvoid*)e.offset);
                glEnableVertexAttribArray(attribIndex++);
            }
        }

        Unbind();
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
    }
}