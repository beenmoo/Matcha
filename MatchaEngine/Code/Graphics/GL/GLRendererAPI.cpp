#include "GLRendererAPI.h"

#include "GLFrameBuffer.h"
#include "GLIndexBuffer.h"
#include "GLShader.h"
#include "GLTexture.h"
#include "GLUniformBuffer.h"
#include "GLVertexArray.h"
#include "GLVertexBuffer.h"

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

std::unique_ptr<Texture> GLRendererAPI::CreateTexture(uint32_t width, uint32_t height)
{
    return std::make_unique<GLTexture>(width, height);
}

std::unique_ptr<Texture> GLRendererAPI::CreateTexture(std::string_view path)
{
    return std::make_unique<GLTexture>(path);
}

std::unique_ptr<Shader> GLRendererAPI::CreateShader(std::string_view name, std::span<const std::string> paths)
{
    return std::make_unique<GLShader>(name, paths);
}

std::unique_ptr<VertexArray> GLRendererAPI::CreateVertexArray()
{
    return std::make_unique<GLVertexArray>();
}

std::shared_ptr<IndexBuffer> GLRendererAPI::CreateIndexBuffer(const uint32_t* indices, uint32_t count)
{
    return std::make_shared<GLIndexBuffer>(indices, count);
}

std::shared_ptr<VertexBuffer> GLRendererAPI::CreateVertexBuffer(const float* vertices, uint32_t sizeInBytes)
{
    return std::make_shared<GLVertexBuffer>(vertices, sizeInBytes);
}

std::unique_ptr<UniformBuffer> GLRendererAPI::CreateUniformBuffer(uint32_t size, uint32_t binding)
{
    return std::make_unique<GLUniformBuffer>(size, binding);
}

std::unique_ptr<FrameBuffer> GLRendererAPI::CreateFrameBuffer(const FrameBufferSpecification& spec)
{
    return std::make_unique<GLFrameBuffer>(spec);
}
}  // namespace Matcha
