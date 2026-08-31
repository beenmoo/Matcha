#pragma once

#include <cstdint>
#include <memory>

namespace Matcha
{
class IndexBuffer
{
public:
    virtual ~IndexBuffer() = default;

    [[nodiscard]] virtual uint32_t GetHandle() const = 0;
    [[nodiscard]] virtual uint32_t GetCount() const = 0;

    [[nodiscard]] static std::shared_ptr<IndexBuffer> Create(const uint32_t* indices, uint32_t count);
};
}  // namespace Matcha
