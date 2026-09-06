#pragma once

#include "Graphics/IndexBuffer.h"

#include <glad/glad.h>

namespace Matcha
{
class GLIndexBuffer final : public IndexBuffer
{
public:
    GLIndexBuffer(const GLuint* indices, GLuint count);
    ~GLIndexBuffer() override;

    GLIndexBuffer(const GLIndexBuffer&) = delete;
    GLIndexBuffer& operator=(const GLIndexBuffer&) = delete;

    [[nodiscard]] uint32_t GetCount() const override;

    // Not part of the abstract IndexBuffer interface - only GLVertexArray, which works with
    // GLIndexBuffer directly, needs the underlying GL buffer object.
    [[nodiscard]] uint32_t GetHandle() const;

private:
    GLuint m_Handle;
    GLuint m_Count;
};
}  // namespace Matcha
