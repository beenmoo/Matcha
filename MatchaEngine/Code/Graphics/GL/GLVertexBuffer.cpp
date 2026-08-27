#include "GLVertexBuffer.h"
#include "GLShaderUtils.h"

namespace Matcha
{
BufferLayout::BufferElement::BufferElement(ShaderDataType type, GLboolean normalized)
    : type(type),
      size(Utils::ShaderDataTypeSize(type)),
      normalized(normalized)
{
}

[[nodiscard]] GLuint BufferLayout::BufferElement::GetComponentCount() const
{
    switch (type)
    {
    case ShaderDataType::Float:
        return 1;
    case ShaderDataType::Float2:
        return 2;
    case ShaderDataType::Float3:
        return 3;
    case ShaderDataType::Float4:
        return 4;
    case ShaderDataType::Mat3:
        return 3;
    case ShaderDataType::Mat4:
        return 4;
    case ShaderDataType::Int:
        return 1;
    case ShaderDataType::Int2:
        return 2;
    case ShaderDataType::Int3:
        return 3;
    case ShaderDataType::Int4:
        return 4;
    case ShaderDataType::Bool:
        return 1;
    default:
        break;
    }

    return 0;
}

BufferLayout::BufferLayout(std::initializer_list<ShaderDataType> dataTypes, GLboolean normalized)
{
    for (const auto& i : dataTypes)
        m_Elements.emplace_back(BufferElement(i, normalized));

    CalculateOffsetsAndStride();
}

[[nodiscard]] const std::vector<BufferLayout::BufferElement>& BufferLayout::GetElements() const
{
    return m_Elements;
}

[[nodiscard]] uint32_t BufferLayout::GetStride() const
{
    return m_Stride;
}

void BufferLayout::CalculateOffsetsAndStride()
{
    size_t offset = 0;
    m_Stride = 0;
    for (auto& element : m_Elements)
    {
        element.offset = offset;
        offset += element.size;
        m_Stride += element.size;
    }
}

GLVertexBuffer::GLVertexBuffer(const GLfloat* vertices, GLuint sizeInBytes)
    : m_SizeInBytes(sizeInBytes)
{
    glCreateBuffers(1, &m_Handle);
    glNamedBufferData(m_Handle, sizeInBytes, vertices, GL_STATIC_DRAW);
}

GLVertexBuffer::~GLVertexBuffer()
{
    glDeleteBuffers(1, &m_Handle);
}

uint32_t GLVertexBuffer::GetSizeInBytes() const
{
    return m_SizeInBytes;
}

void GLVertexBuffer::SetLayout(const std::shared_ptr<BufferLayout> layout)
{
    m_Layout = layout;
}

const BufferLayout* GLVertexBuffer::GetLayout() const
{
    return m_Layout.get();
}

GLuint GLVertexBuffer::GetHandle() const
{
    return m_Handle;
}
}  // namespace Matcha