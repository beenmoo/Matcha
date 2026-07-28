#pragma once

#include "Types.h"

namespace Matcha
{
    class Time
    {
    public:
        void Update();

        float GetDeltaTime() const;

    private:
        uint64 mLastFrameTime = 0;
        uint64 mElapsedTime = 0;
        float mDeltaTime = 0.0f;
    };
}