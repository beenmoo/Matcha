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

BufferLayout::BufferLayout(std::initializer_list<ShaderDataType> dataTypes, GLboolean normalized = false)
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

GLVertexBuffer::GLVertexBuffer(GLuint sizeInBytes)
{
    glCreateBuffers(1, &m_Handle);
    glNamedBufferData(m_Handle, sizeInBytes, nullptr, GL_DYNAMIC_DRAW);
}

GLVertexBuffer::GLVertexBuffer(const GLfloat* vertices, GLuint sizeInBytes)
{
    glCreateBuffers(1, &m_Handle);
    glNamedBufferData(m_Handle, sizeInBytes, vertices, GL_STATIC_DRAW);

    for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
        m_Vertices.emplace_back(vertices[i]);
}

GLVertexBuffer::~GLVertexBuffer()
{
    glDeleteBuffers(1, &m_Handle);
}

void GLVertexBuffer::AddVertex(std::initializer_list<GLfloat> vertex)
{
    m_Vertices.insert(m_Vertices.end(), vertex.begin(), vertex.end());

    glNamedBufferData(m_Handle, GetSizeInBytes(), m_Vertices.data(), m_DrawType);
}

void GLVertexBuffer::SetVertices(const GLfloat* vertices, GLuint sizeInBytes)
{
    glNamedBufferSubData(m_Handle, 0, sizeInBytes, vertices);

    Clear();

    for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
        m_Vertices.emplace_back(vertices[i]);
}

void GLVertexBuffer::SetVerticesNew(const GLfloat* vertices, GLuint sizeInBytes)
{
    glNamedBufferData(m_Handle, sizeInBytes, vertices, m_DrawType);

    Clear();

    for (size_t i = 0; i < sizeInBytes / sizeof(GLfloat); ++i)
        m_Vertices.emplace_back(vertices[i]);
}

void GLVertexBuffer::SetDrawType(GLenum drawType)
{
    m_DrawType = drawType;
}

uint32_t GLVertexBuffer::GetSizeInBytes() const
{
    return static_cast<GLuint>(m_Vertices.size() * sizeof(GLfloat));
}

void GLVertexBuffer::Clear()
{
    m_Vertices.clear();
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