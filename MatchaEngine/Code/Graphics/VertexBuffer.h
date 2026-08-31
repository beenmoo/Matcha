#pragma once

#include "Graphics/ShaderDataType.h"

#include <cstdint>
#include <initializer_list>
#include <memory>
#include <vector>

namespace Matcha
{
class BufferLayout
{
private:
    struct BufferElement
    {
        ShaderDataType type = ShaderDataType::None;
        uint32_t size = 0;
        size_t offset = 0;
        bool normalized = false;

        BufferElement(ShaderDataType type, bool normalized = false);

        [[nodiscard]] uint32_t GetComponentCount() const;
    };

public:
    BufferLayout(std::initializer_list<ShaderDataType> dataTypes, bool normalized = false);

    [[nodiscard]] const std::vector<BufferElement>& GetElements() const;
    [[nodiscard]] uint32_t GetStride() const;

private:
    void CalculateOffsetsAndStride();

private:
    std::vector<BufferElement> m_Elements;

    uint32_t m_Stride = 0;
};

class VertexBuffer
{
public:
    virtual ~VertexBuffer() = default;

    virtual void SetLayout(const std::shared_ptr<BufferLayout> layout) = 0;
    [[nodiscard]] virtual const BufferLayout* GetLayout() const = 0;

    [[nodiscard]] virtual uint32_t GetHandle() const = 0;
    [[nodiscard]] virtual uint32_t GetSizeInBytes() const = 0;

    [[nodiscard]] static std::shared_ptr<VertexBuffer> Create(const float* vertices, uint32_t sizeInBytes);
};
}  // namespace Matcha
