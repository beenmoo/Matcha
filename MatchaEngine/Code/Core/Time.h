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
    std::uint64_t mLastFrameTime = 0;
    std::uint64_t mElapsedTime = 0;
    float mDeltaTime = 0.0f;
};
}  // namespace Matcha