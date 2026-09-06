#pragma once

#include <cstdint>
#include <memory>

namespace Matcha
{
class IndexBuffer
{
public:
    virtual ~IndexBuffer() = default;

    [[nodiscard]] virtual uint32_t GetCount() const = 0;

};
}  // namespace Matcha
