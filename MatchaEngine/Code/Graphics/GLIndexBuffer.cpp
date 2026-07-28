#include "GLIndexBuffer.h"

namespace Matcha
{
    GLIndexBuffer::GLIndexBuffer(GLuint* indices, GLuint count)
    {
        glCreateBuffers(1, &mObjectID);
        
        Bind();
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
        Unbind();

        for (size_t i = 0; i < count; ++i)
            mIndices.emplace_back(indices[i]);
    }

    GLIndexBuffer::~GLIndexBuffer()
    {
        glDeleteBuffers(1, &mObjectID);
    }

    void GLIndexBuffer::Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mObjectID);
    }

    void GLIndexBuffer::Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void GLIndexBuffer::AddIndices(std::initializer_list<GLuint> indices)
    {
        mIndices.insert(mIndices.end(), indices.begin(), indices.end());

        Bind();
        glBufferData(GL_ARRAY_BUFFER, mIndices.size() * sizeof(GLuint), mIndices.data(), mDrawType);
        Unbind();
    }

    void GLIndexBuffer::SetIndices(GLuint* indices, GLuint count)
    {
        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(GLuint), indices);
        Unbind();

        Clear();

        for (size_t i = 0; i < count; ++i)
            mIndices.emplace_back(indices[i]);
    }

    void GLIndexBuffer::SetIndicesNew(GLuint* indices, GLuint count)
    {
        Bind();
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(GLuint), indices, GL_STATIC_DRAW);
        Unbind();

        Clear();

        for (size_t i = 0; i < count; ++i)
            mIndices.emplace_back(indices[i]);
    }

    void GLIndexBuffer::Clear()
    {
        mIndices.clear();
    }

    uint32_t GLIndexBuffer::GetCount() const
    {
        return static_cast<GLuint>(mIndices.size());
    }
}