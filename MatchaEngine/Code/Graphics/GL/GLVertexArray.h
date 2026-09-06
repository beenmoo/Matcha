#pragma once

#include "GLIndexBuffer.h"
#include "GLVertexBuffer.h"
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

    GLVertexArray(const GLVertexArray&) = delete;
    GLVertexArray& operator=(const GLVertexArray&) = delete;

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

    // Concrete GL types, not the abstract VertexBuffer/IndexBuffer: a GL VAO can only ever bind GL
    // buffer objects (GetHandle() lives only on the GL classes now, not the abstract interfaces -
    // see VertexBuffer.h/IndexBuffer.h), and every buffer reaching AddVertexBuffer/SetIndexBuffer
    // below was created by this same GL backend, so narrowing here is safe.
    std::vector<std::shared_ptr<GLVertexBuffer>> m_VertexBuffers;
    std::shared_ptr<GLIndexBuffer> m_IndexBuffer = nullptr;
};
}  // namespace Matcha
