#pragma once

#include <cstdint>
#include <functional>

namespace Matcha
{
class UUID
{
public:
    UUID();
    UUID(uint64_t uuid);
    UUID(const UUID&) = default;

    // Allow implicit casting to uint64_t for easy use in maps and comparisons
    operator uint64_t() const
    {
        return m_UUID;
    }

private:
    uint64_t m_UUID;
};
}  // namespace Matcha

// Hash specialization so you can use UUID as a key in std::unordered_map / entt
namespace std
{
template <>
struct hash<Matcha::UUID>
{
    size_t operator()(const Matcha::UUID& uuid) const
    {
        return (uint64_t)uuid;
    }
};
}  // namespace std