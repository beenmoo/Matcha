#include "GLIndexBuffer.h"

namespace Matcha
{
GLIndexBuffer::GLIndexBuffer(GLuint* indices, GLuint count)
{
    glCreateBuffers(1, &mHandle);
    glNamedBufferData(mHandle, count * sizeof(GLuint), indices, GL_STATIC_DRAW);

    for (size_t i = 0; i < count; ++i)
        mIndices.emplace_back(indices[i]);
}

GLIndexBuffer::~GLIndexBuffer()
{
    glDeleteBuffers(1, &mHandle);
}

void GLIndexBuffer::AddIndices(std::initializer_list<GLuint> indices)
{
    mIndices.insert(mIndices.end(), indices.begin(), indices.end());

    glNamedBufferData(mHandle, mIndices.size() * sizeof(GLuint), mIndices.data(), mDrawType);
}

void GLIndexBuffer::SetIndices(GLuint* indices, GLuint count)
{
    glNamedBufferSubData(mHandle, 0, count * sizeof(GLuint), indices);

    Clear();

    for (size_t i = 0; i < count; ++i)
        mIndices.emplace_back(indices[i]);
}

void GLIndexBuffer::SetIndicesNew(GLuint* indices, GLuint count)
{
    glNamedBufferData(mHandle, count * sizeof(GLuint), indices, GL_STATIC_DRAW);

    Clear();

    for (size_t i = 0; i < count; ++i)
        mIndices.emplace_back(indices[i]);
}

void GLIndexBuffer::Clear()
{
    mIndices.clear();
}

GLuint GLIndexBuffer::GetHandle() const
{
    return mHandle;
}

GLuint GLIndexBuffer::GetCount() const
{
    return static_cast<GLuint>(mIndices.size());
}
}  // namespace Matcha