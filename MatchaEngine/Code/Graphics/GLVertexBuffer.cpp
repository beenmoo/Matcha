#include "GLVertexBuffer.h"

namespace Matcha
{
    GLVertexBuffer::GLVertexBuffer(GLuint sizeInBytes)
    {
        glCreateBuffers(1, &mObjectID);

        Bind();
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, nullptr, GL_DYNAMIC_DRAW);
        Unbind();
    }

    GLVertexBuffer::GLVertexBuffer(const GLfloat* vertices, GLuint sizeInBytes)
    {
        glCreateBuffers(1, &mObjectID);

        Bind();
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices, GL_STATIC_DRAW);
        Unbind();

        for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
            mVertices.emplace_back(vertices[i]);
    }

    GLVertexBuffer::~GLVertexBuffer()
    {
        glDeleteBuffers(1, &mObjectID);
    }

    void GLVertexBuffer::Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, mObjectID);
    }

    void GLVertexBuffer::Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    void GLVertexBuffer::AddVertex(std::initializer_list<GLfloat> vertex)
    {
        mVertices.insert(mVertices.end(), vertex.begin(), vertex.end());

        Bind();
        glBufferData(GL_ARRAY_BUFFER, GetSizeInBytes(), mVertices.data(), mDrawType);
        Unbind();
    }
    
    void GLVertexBuffer::SetVertices(const GLfloat* vertices, GLuint sizeInBytes)
    {
        Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeInBytes, vertices);
        Unbind();

        Clear();

        for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
            mVertices.emplace_back(vertices[i]);
    }

    void GLVertexBuffer::SetVerticesNew(const GLfloat* vertices, GLuint sizeInBytes)
    {
        Bind();
        glBufferData(GL_ARRAY_BUFFER, sizeInBytes, vertices, mDrawType);
        Unbind();

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
}