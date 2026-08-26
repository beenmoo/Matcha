#pragma once

#include <vector>
#include <glad/glad.h>

namespace Matcha
{
class GLIndexBuffer
{
public:
    GLIndexBuffer(const GLuint* indices, GLuint count);
    ~GLIndexBuffer();

    void AddIndices(std::initializer_list<GLuint> indices);
    void SetIndices(const GLuint* indices, GLuint count);
    void SetIndicesNew(const GLuint* indices, GLuint count);
    void Clear();

    [[nodiscard]] GLuint GetHandle() const;
    [[nodiscard]] GLuint GetCount() const;

private:
    GLuint mHandle;
    GLenum mDrawType = GL_STATIC_DRAW;
    std::vector<GLuint> mIndices;
};
}  // namespace Matcha