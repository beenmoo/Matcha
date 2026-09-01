#pragma once

#include <cstdint>

namespace Matcha
{
enum class EventType
{
    Quit,
    WindowResized,
    WindowFocusLost,
    MouseMoved,
    MouseScrolled,
    JoystickMoved
};

struct Event
{
    EventType type;

    int32_t width = 0, height = 0;
    float x = 0.0f, y = 0.0f;
    uint8_t axis = 0;
};
}  // namespace Matcha
