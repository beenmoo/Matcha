#include "GLRendererAPI.h"

#include <glad/glad.h>

namespace Matcha
{
void GLRendererAPI::Init()
{
    glEnable(GL_DEPTH_TEST);
}

void GLRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    glViewport(static_cast<GLint>(x), static_cast<GLint>(y), static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void GLRendererAPI::SetClearColor(const Vector4& color)
{
    glClearColor(color.x, color.y, color.z, color.w);
}

void GLRendererAPI::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GLRendererAPI::DrawIndexed(const VertexArray& vertexArray, uint32_t indexCount)
{
    vertexArray.Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, nullptr);
}
}  // namespace Matcha
