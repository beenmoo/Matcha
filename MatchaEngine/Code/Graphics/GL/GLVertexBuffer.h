#pragma once

#include "Math/Vector.h"
#include "GLShaderUtils.h"

#include <vector>
#include <initializer_list>

namespace Matcha
{
class GLVertexBuffer
{
public:
    class BufferLayout
    {
    private:
        struct BufferElement
        {
            Utils::ShaderDataType type = Utils::ShaderDataType::None;
            GLuint size = 0;
            size_t offset = 0;
            GLboolean normalized = false;

            BufferElement(Utils::ShaderDataType type, GLboolean normalized = false) : type(type),
                                                                                            size(Utils::ShaderDataTypeSize(type)),
                                                                                            normalized(normalized)
            {
            }

            [[nodiscard]] uint32_t GetComponentCount() const
            {
                switch (type)
                {
                case Utils::ShaderDataType::Float:
                    return 1;
                case Utils::ShaderDataType::Float2:
                    return 2;
                case Utils::ShaderDataType::Float3:
                    return 3;
                case Utils::ShaderDataType::Float4:
                    return 4;
                case Utils::ShaderDataType::Mat3:
                    return 3;
                case Utils::ShaderDataType::Mat4:
                    return 4;
                case Utils::ShaderDataType::Int:
                    return 1;
                case Utils::ShaderDataType::Int2:
                    return 2;
                case Utils::ShaderDataType::Int3:
                    return 3;
                case Utils::ShaderDataType::Int4:
                    return 4;
                case Utils::ShaderDataType::Bool:
                    return 1;
                default:
                    break;
                }

                return 0;
            }
        };

    public:
        BufferLayout(std::initializer_list<Utils::ShaderDataType> dataTypes, GLboolean normalized = false)
        {
            for (const auto& i : dataTypes)
                m_Elements.emplace_back(BufferElement(i, normalized));

            CalculateOffsetsAndStride();
        }

        [[nodiscard]] const std::vector<BufferElement>& GetElements() const
        {
            return m_Elements;
        }

        [[nodiscard]] uint32_t GetStride() const
        {
            return m_Stride;
        }

    private:
        void CalculateOffsetsAndStride()
        {
            size_t offset = 0;
            m_Stride = 0;

            for (auto& e : m_Elements)
            {
                e.offset += offset;
                offset += e.offset;
                m_Stride += e.size;
            }
        }

    private:
        std::vector<BufferElement> m_Elements;

        GLsizei m_Stride = 0;
    };

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