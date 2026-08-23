#include "GLVertexBuffer.h"

namespace Matcha
{
GLVertexBuffer::GLVertexBuffer(GLuint sizeInBytes)
{
    glCreateBuffers(1, &mHandle);
    glNamedBufferData(mHandle, sizeInBytes, nullptr, GL_DYNAMIC_DRAW);
}

GLVertexBuffer::GLVertexBuffer(const GLfloat* vertices, GLuint sizeInBytes)
{
    glCreateBuffers(1, &mHandle);
    glNamedBufferData(mHandle, sizeInBytes, vertices, GL_STATIC_DRAW);

    for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
        mVertices.emplace_back(vertices[i]);
}

GLVertexBuffer::~GLVertexBuffer()
{
    glDeleteBuffers(1, &mHandle);
}

void GLVertexBuffer::AddVertex(std::initializer_list<GLfloat> vertex)
{
    mVertices.insert(mVertices.end(), vertex.begin(), vertex.end());

    glNamedBufferData(mHandle, GetSizeInBytes(), mVertices.data(), mDrawType);
}

void GLVertexBuffer::SetVertices(const GLfloat* vertices, GLuint sizeInBytes)
{
    glNamedBufferSubData(mHandle, 0, sizeInBytes, vertices);

    Clear();

    for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
        mVertices.emplace_back(vertices[i]);
}

void GLVertexBuffer::SetVerticesNew(const GLfloat* vertices, GLuint sizeInBytes)
{
    glNamedBufferData(mHandle, sizeInBytes, vertices, mDrawType);

    Clear();

    for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
        mVertices.emplace_back(vertices[i]);
}

void GLVertexBuffer::SetDrawType(GLenum drawType)
{
    mDrawType = drawType;
}

uint32_t GLVertexBuffer::GetSizeInBytes() const
{
    return static_cast<GLuint>(mVertices.size() * sizeof(GLfloat));
}

void GLVertexBuffer::Clear()
{
    mVertices.clear();
}

void GLVertexBuffer::SetLayout(const std::shared_ptr<BufferLayout> layout)
{
    mLayout = layout;
}

const GLVertexBuffer::BufferLayout* GLVertexBuffer::GetLayout() const
{
    return mLayout.get();
}

GLuint GLVertexBuffer::GetHandle() const
{
    return mHandle;
}
}  // namespace Matcha