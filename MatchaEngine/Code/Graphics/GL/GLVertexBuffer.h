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

    GLVertexBuffer(const GLVertexBuffer&) = delete;
    GLVertexBuffer& operator=(const GLVertexBuffer&) = delete;

    void SetLayout(const std::shared_ptr<BufferLayout> layout) override;
    [[nodiscard]] const BufferLayout* GetLayout() const override;

    [[nodiscard]] uint32_t GetSizeInBytes() const override;

    // Not part of the abstract VertexBuffer interface - only GLVertexArray, which works with
    // GLVertexBuffer directly, needs the underlying GL buffer object.
    [[nodiscard]] uint32_t GetHandle() const;

private:
    GLuint m_Handle;
    GLuint m_SizeInBytes;

    std::shared_ptr<BufferLayout> m_Layout = nullptr;
};
}  // namespace Matcha
