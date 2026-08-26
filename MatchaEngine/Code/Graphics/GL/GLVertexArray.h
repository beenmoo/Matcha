#pragma once

#include <vector>
#include <memory>

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
    GLuint mHandle;

    std::vector<std::shared_ptr<GLVertexBuffer>> mVertexBuffers;
    std::shared_ptr<GLIndexBuffer> mIndexBuffer = nullptr;
};
}  // namespace Matcha