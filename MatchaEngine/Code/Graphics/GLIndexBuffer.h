#pragma once

#include <vector>
#include <glad/glad.h>

namespace Matcha
{
    class GLIndexBuffer
    {
    public:
        GLIndexBuffer(GLuint* indices, GLuint count);
        ~GLIndexBuffer();

        void Bind() const;
        void Unbind() const;

        void AddIndices(std::initializer_list<GLuint> indices);
        void SetIndices(GLuint* indices, GLuint count);
        void SetIndicesNew(GLuint* indices, GLuint count);
        void Clear();

        GLuint GetCount() const;

    private:
        GLuint mObjectID;

        GLenum mDrawType = GL_STATIC_DRAW;

        std::vector<GLuint> mIndices;
    };
}