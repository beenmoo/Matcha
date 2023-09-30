#include "VertexArray.h"
#include "Core/Core.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

namespace Matcha
{
    VertexArray::VertexArray()
    {
        glCreateVertexArrays(1, &mObjectID);
    }
    
    VertexArray::~VertexArray()
    {
        glDeleteVertexArrays(1, &mObjectID);
    }
    
    void VertexArray::Bind() const
    {
        glBindVertexArray(mObjectID);
    }
    
    void VertexArray::Unbind() const
    {
        glBindVertexArray(0);
    }
    
    void VertexArray::InitAttributes(uint32_t vbIndex)
    {
        MT_ASSERT(vbIndex < mVertexBuffers.size(), "Index out of range!");

        Bind();

        const auto& buffer = mVertexBuffers[vbIndex];
        buffer->Bind();

        uint32_t attribIndex = 0;

        auto layout = buffer->GetLayout();

        for (const auto& e : layout->GetElements())
        {
            if (e.mType == ShaderUtils::ShaderDataType::Mat3 ||
                e.mType == ShaderUtils::ShaderDataType::Mat4)
            {
                uint8_t count = e.GetComponentCount();

                for (uint8_t i = 0; i < count; i++)
                {
                    glVertexAttribPointer(attribIndex,
                                          count,
                                          ShaderUtils::ShaderDataTypeToGLDataType(e.mType),
                                          e.mNormalized,
                                          layout->GetStride(),
                                          (const void*)(e.mOffset + sizeof(float) * count * i));
                    glVertexAttribDivisor(attribIndex, 1);
                    glEnableVertexAttribArray(attribIndex++);
                }
            }
            else
            {
                glVertexAttribPointer(attribIndex,
                                      e.GetComponentCount(),
                                      ShaderUtils::ShaderDataTypeToGLDataType(e.mType),
                                      e.mNormalized,
                                      layout->GetStride(),
                                      (const void*)e.mOffset);
                glEnableVertexAttribArray(attribIndex++);
            }
        }

        Unbind();
    }

    void VertexArray::AddVertexBuffer(const std::shared_ptr<VertexBuffer>& buffer)
    {
        MT_ASSERT(buffer->GetLayout(), "Vertex Buffer has no layout!");

        mVertexBuffers.emplace_back(buffer);

        InitAttributes(static_cast<uint32_t>(mVertexBuffers.size() - 1));
    }
    
    void VertexArray::SetIndexBuffer(const std::shared_ptr<IndexBuffer>& buffer)
    {
        mIndexBuffer = buffer;
    }
}