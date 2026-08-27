#pragma once

#include "Graphics/ShaderDataType.h"

#include <glad/glad.h>
#include <vector>
#include <initializer_list>

namespace Matcha
{
class BufferLayout
{
private:
    struct BufferElement
    {
        ShaderDataType type = ShaderDataType::None;
        GLuint size = 0;
        size_t offset = 0;
        GLboolean normalized = false;

        BufferElement(ShaderDataType type, GLboolean normalized = false);

        [[nodiscard]] GLuint GetComponentCount() const;
    };

public:
    BufferLayout(std::initializer_list<ShaderDataType> dataTypes, GLboolean normalized = false);

    [[nodiscard]] const std::vector<BufferElement>& GetElements() const;
    [[nodiscard]] uint32_t GetStride() const;

private:
    void CalculateOffsetsAndStride();

private:
    std::vector<BufferElement> m_Elements;

    GLsizei m_Stride = 0;
};

class GLVertexBuffer
{
public:
    GLVertexBuffer(GLuint sizeInBytes = 0);
    GLVertexBuffer(const GLfloat* vertices, GLuint sizeInBytes);
    ~GLVertexBuffer();

    void AddVertex(std::initializer_list<GLfloat> vertex);
    void SetVertices(const GLfloat* vertices, GLuint sizeInBytes);
    void SetVerticesNew(const GLfloat* vertices, GLuint sizeInBytes);
    void SetDrawType(GLenum drawType);
    void Clear();

    void SetLayout(const std::shared_ptr<BufferLayout> layout);
    [[nodiscard]] const BufferLayout* GetLayout() const;

    [[nodiscard]] GLuint GetHandle() const;
    [[nodiscard]] GLuint GetSizeInBytes() const;

private:
    GLuint m_Handle;

    GLenum m_DrawType = GL_STATIC_DRAW;

    std::shared_ptr<BufferLayout> m_Layout = nullptr;
    std::vector<GLfloat> m_Vertices;
};
}  // namespace Matcha