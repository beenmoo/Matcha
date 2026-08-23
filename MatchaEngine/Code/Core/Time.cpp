#include "Time.h"

#include <SDL3/SDL.h>

namespace Matcha
{
void Time::Update()
{
    mLastFrameTime = mElapsedTime;
    mElapsedTime = SDL_GetPerformanceCounter();
    mDeltaTime = static_cast<float>((mElapsedTime - mLastFrameTime) * 1000.0f /
                                    static_cast<float>(SDL_GetPerformanceFrequency())) *
                 0.001f;
}

float Time::GetDeltaTime() const
{
    return mDeltaTime;
}
}  // namespace Matcha