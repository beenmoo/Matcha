#pragma once

#include "Graphics/ShaderDataType.h"
#include "Graphics/VertexArray.h"

#include <glad/glad.h>
#include <vector>

namespace Matcha
{
class GLVertexArray final : public VertexArray
{
public:
    GLVertexArray();
    ~GLVertexArray() override;

    void Bind() const override;
    void Unbind() const override;

    void AddVertexBuffer(const std::shared_ptr<VertexBuffer> buffer) override;
    void SetIndexBuffer(const std::shared_ptr<IndexBuffer> buffer) override;

private:
    void InitAttributes(GLuint vbIndex);
    void BindAttribute(GLuint attribIndex, GLuint bufferHandle, GLsizei stride, GLint componentCount, ShaderDataType type,
                       bool normalized, GLuint offset);

private:
    GLuint m_Handle;

    std::vector<std::shared_ptr<VertexBuffer>> m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer = nullptr;
};
}  // namespace Matcha
