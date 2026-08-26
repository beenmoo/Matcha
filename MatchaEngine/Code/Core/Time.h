#pragma once

#include <cstdint>

namespace Matcha
{
class Time
{
public:
    void Update();

    [[nodiscard]] float GetDeltaTime() const;

private:
    std::uint64_t m_LastFrameTime = 0;
    std::uint64_t m_ElapsedTime = 0;
    float m_DeltaTime = 0.0f;
};
}  // namespace Matcha