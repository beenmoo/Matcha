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

    [[nodiscard]] uint32_t GetHandle() const override;
    [[nodiscard]] uint32_t GetCount() const override;

private:
    GLuint m_Handle;
    GLuint m_Count;
};
}  // namespace Matcha
