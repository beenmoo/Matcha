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

    void AddIndices(std::initializer_list<GLuint> indices);
    void SetIndices(GLuint* indices, GLuint count);
    void SetIndicesNew(GLuint* indices, GLuint count);
    void Clear();

    [[nodiscard]] GLuint GetHandle() const;
    [[nodiscard]] GLuint GetCount() const;

private:
    GLuint mHandle;
    GLenum mDrawType = GL_STATIC_DRAW;
    std::vector<GLuint> mIndices;
};
}  // namespace Matcha