#include "Time.h"

#include <sdl/SDL.h>

namespace Matcha
{
    void Time::Update()
    {
        mLastFrameTime = mElapsedTime;
        mElapsedTime = SDL_GetPerformanceCounter();
        mDeltaTime = static_cast<float>((mElapsedTime - mLastFrameTime) * 1000.0f /
                                        static_cast<float>(SDL_GetPerformanceFrequency())) * 0.001f;
    }
    
    float Time::GetDeltaTime() const
    {
        return mDeltaTime;
    }
}