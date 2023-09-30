#include "VertexBuffer.h"

namespace Matcha
{
    VertexBuffer::VertexBuffer(uint32_t sizeInBytes)
    {
        glCreateBuffers(1, &mObjectID);

        Bind();
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, nullptr, GL_DYNAMIC_DRAW);
        Unbind();
    }

    VertexBuffer::VertexBuffer(const float* vertices, uint32_t sizeInBytes)
    {
        glCreateBuffers(1, &mObjectID);

        Bind();
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices, GL_STATIC_DRAW);
        Unbind();

        for (size_t i = 0; i < sizeInBytes / sizeof(float); ++i)
            mVertices.emplace_back(vertices[i]);
    }

    VertexBuffer::~VertexBuffer()
    {
        glDeleteBuffers(1, &mObjectID);
    }

    void VertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, mObjectID);
    }

    void VertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    void VertexBuffer::AddVertex(std::initializer_list<float> vertex)
    {
        mVertices.insert(mVertices.end(), vertex.begin(), vertex.end());

        Bind();
        glBufferData(GL_ARRAY_BUFFER, GetSizeInBytes(), mVertices.data(), mDrawType);
        Unbind();
    }
    
    void VertexBuffer::SetVertices(const float* vertices, uint32_t sizeInBytes)
    {
        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, vertices);
        Unbind();

        Clear();

        for (size_t i = 0; i < sizeInBytes / sizeof(float); ++i)
            mVertices.emplace_back(vertices[i]);
    }

    void VertexBuffer::SetVerticesNew(const float* vertices, uint32_t sizeInBytes)
    {
        Bind();
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices, mDrawType);
        Unbind();

        Clear();

        for (size_t i = 0; i < sizeInBytes / sizeof(float); ++i)
            mVertices.emplace_back(vertices[i]);
    }

    void VertexBuffer::SetDrawType(GLenum drawType)
    {
        mDrawType = drawType;
    }
    
    uint32_t VertexBuffer::GetSizeInBytes() const
    {
        return static_cast<uint32_t>(mVertices.size() * sizeof(float));
    }
    
    void VertexBuffer::Clear()
    {
        mVertices.clear();
    }
    
    void VertexBuffer::SetLayout(const std::shared_ptr<BufferLayout>& layout)
    {
        mLayout = layout;
    }

    const VertexBuffer::BufferLayout* VertexBuffer::GetLayout() const
    {
        return mLayout.get();
    }
}