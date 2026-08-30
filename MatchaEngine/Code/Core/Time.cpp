#include "Time.h"

namespace Matcha
{
void Time::Update()
{
    auto now = std::chrono::steady_clock::now();
    m_DeltaTime = std::chrono::duration<float>(now - m_LastFrameTime).count();
    m_LastFrameTime = now;
}

float Time::GetDeltaTime() const
{
    return m_DeltaTime;
}
}  // namespace Matcha