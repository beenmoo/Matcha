#include "IndexBuffer.h"

namespace Matcha
{
    IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
    {
        glCreateBuffers(1, &mObjectID);
        
        Bind();
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
        Unbind();

        for (size_t i = 0; i < count; ++i)
            mIndices.emplace_back(indices[i]);
    }

    IndexBuffer::~IndexBuffer()
    {
        glDeleteBuffers(1, &mObjectID);
    }

    void IndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mObjectID);
    }

    void IndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void IndexBuffer::AddIndices(std::initializer_list<uint32_t> indices)
    {
        mIndices.insert(mIndices.end(), indices.begin(), indices.end());

        Bind();
        glBufferData(GL_ARRAY_BUFFER, mIndices.size() * sizeof(uint32_t), mIndices.data(), mDrawType);
        Unbind();
    }

    void IndexBuffer::SetIndices(uint32_t* indices, uint32_t count)
    {
        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(uint32_t), indices);
        Unbind();

        Clear();

        for (size_t i = 0; i < count; ++i)
            mIndices.emplace_back(indices[i]);
    }

    void IndexBuffer::SetIndicesNew(uint32_t* indices, uint32_t count)
    {
        Bind();
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
        Unbind();

        Clear();

        for (size_t i = 0; i < count; ++i)
            mIndices.emplace_back(indices[i]);
    }

    void IndexBuffer::Clear()
    {
        mIndices.clear();
    }

    uint32_t IndexBuffer::GetCount() const
    {
        return static_cast<uint32_t>(mIndices.size());
    }
}