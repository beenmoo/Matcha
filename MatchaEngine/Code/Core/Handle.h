#pragma once

#include <cstdint>

namespace Matcha
{
template <typename Tag>
class Handle
{
public:
    constexpr Handle() = default;
    constexpr explicit Handle(uint32_t id)
        : m_ID(id)
    {
    }

    [[nodiscard]] constexpr uint32_t GetID() const
    {
        return m_ID;
    }

    [[nodiscard]] constexpr bool IsValid() const
    {
        return m_ID != 0;
    }

    constexpr bool operator==(const Handle&) const = default;

private:
    uint32_t m_ID = 0;
};
}  // namespace Matcha
