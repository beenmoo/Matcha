#pragma once

#include <cstdint>

namespace Matcha
{
template <typename Tag>
class Handle
{
public:
    constexpr Handle() = default;
    constexpr explicit Handle(uint32_t id) : mID(id)
    {
    }

    [[nodiscard]] constexpr uint32_t GetID() const
    {
        return mID;
    }

    [[nodiscard]] constexpr bool IsValid() const
    {
        return mID != 0;
    }

    constexpr bool operator==(const Handle&) const = default;

private:
    uint32_t mID = 0;
};
}  // namespace Matcha
