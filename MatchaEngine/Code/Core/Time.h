#pragma once

#include <chrono>

namespace Matcha
{
class Time
{
public:
    void Update();

    [[nodiscard]] float GetDeltaTime() const;

private:
    // std::chrono::steady_clock rather than a platform timer API (SDL's performance counter,
    // previously) - Time is shared by every window backend, including Qt, which never calls
    // SDL_Init, so it shouldn't depend on SDL for something the standard library already
    // provides portably. Initialized to "now" rather than a zero/epoch value so the very first
    // Update() call reports a real (small) delta instead of a bogus huge one.
    std::chrono::steady_clock::time_point m_LastFrameTime = std::chrono::steady_clock::now();
    float m_DeltaTime = 0.0f;
};
}  // namespace Matcha