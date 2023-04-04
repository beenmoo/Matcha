#pragma once

#include <cstdint>

namespace Matcha
{
    class Time
    {
    public:
        void Update();

        float GetDeltaTime() const;

    private:
        uint64_t mLastFrameTime = 0;
        uint64_t mElapsedTime = 0;
        float mDeltaTime = 0.0f;
    };
}