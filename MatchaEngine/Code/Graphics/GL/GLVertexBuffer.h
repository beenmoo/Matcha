#pragma once

#include "Graphics/VertexBuffer.h"

#include <glad/glad.h>

namespace Matcha
{
class GLVertexBuffer final : public VertexBuffer
{
public:
    GLVertexBuffer(const GLfloat* vertices, GLuint sizeInBytes);
    ~GLVertexBuffer() override;

    void SetLayout(const std::shared_ptr<BufferLayout> layout) override;
    [[nodiscard]] const BufferLayout* GetLayout() const override;

    [[nodiscard]] uint32_t GetHandle() const override;
    [[nodiscard]] uint32_t GetSizeInBytes() const override;

private:
    GLuint m_Handle;
    GLuint m_SizeInBytes;

    std::shared_ptr<BufferLayout> m_Layout = nullptr;
};
}  // namespace Matcha
