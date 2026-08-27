#pragma once

#include <glad/glad.h>

namespace Matcha
{
class GLIndexBuffer
{
public:
    GLIndexBuffer(const GLuint* indices, GLuint count);
    ~GLIndexBuffer();

    [[nodiscard]] GLuint GetHandle() const;
    [[nodiscard]] GLuint GetCount() const;

private:
    GLuint m_Handle;
    GLuint m_Count;
};
}  // namespace Matcha