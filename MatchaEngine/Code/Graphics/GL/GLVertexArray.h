#pragma once

#include <vector>
#include <memory>
#include <glad/glad.h>

namespace Matcha
{
class GLVertexBuffer;
class GLIndexBuffer;

class GLVertexArray
{
public:
    GLVertexArray();
    ~GLVertexArray();

    void Bind() const;
    void Unbind() const;

    void InitAttributes(GLuint vbIndex);
    void AddVertexBuffer(const std::shared_ptr<GLVertexBuffer> buffer);
    void SetIndexBuffer(const std::shared_ptr<GLIndexBuffer> buffer);

private:
    GLuint m_Handle;

    std::vector<std::shared_ptr<GLVertexBuffer>> m_VertexBuffers;
    std::shared_ptr<GLIndexBuffer> m_IndexBuffer = nullptr;
};
}  // namespace Matcha