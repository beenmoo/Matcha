#include "Time.h"

#include <SDL3/SDL.h>

namespace Matcha
{
void Time::Update()
{
    m_LastFrameTime = m_ElapsedTime;
    m_ElapsedTime = SDL_GetPerformanceCounter();
    m_DeltaTime = static_cast<float>((m_ElapsedTime - m_LastFrameTime) * 1000.0f /
                                    static_cast<float>(SDL_GetPerformanceFrequency())) *
                 0.001f;
}

float Time::GetDeltaTime() const
{
    return m_DeltaTime;
}
}  // namespace Matcha