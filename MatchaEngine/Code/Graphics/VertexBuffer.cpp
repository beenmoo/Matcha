#include "VertexBuffer.h"
#include "GL/GLShaderUtils.h"
#include "RendererAPI.h"

namespace Matcha
{
BufferLayout::BufferElement::BufferElement(ShaderDataType type, bool normalized)
    : type(type),
      size(ShaderDataTypeSize(type)),
      normalized(normalized)
{
}

[[nodiscard]] uint32_t BufferLayout::BufferElement::GetComponentCount() const
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

BufferLayout::BufferLayout(std::initializer_list<ShaderDataType> dataTypes, bool normalized)
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

std::shared_ptr<VertexBuffer> VertexBuffer::Create(const float* vertices, uint32_t sizeInBytes)
{
    return GetActiveRendererAPI().CreateVertexBuffer(vertices, sizeInBytes);
}
}  // namespace Matcha
