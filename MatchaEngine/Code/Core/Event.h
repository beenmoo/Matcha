#pragma once

#include "KeyCodes.h"

#include <cstdint>

namespace Matcha
{
// Free-standing (not nested in Input) so Event below can carry one without Event.h depending on
// Input.h - Input.h already depends on Event.h, so the reverse would cycle. Input::MouseButton is
// a type alias back to this, so every existing Input::MouseButton::Left-style call site is
// unaffected.
enum class MouseButton
{
    Left,
    Middle,
    Right,
    Back,
    Forward
};

enum class EventType
{
    Quit,
    WindowResized,
    WindowFocusLost,
    MouseMoved,
    MouseScrolled,
    JoystickMoved,
    KeyDown,
    KeyUp,
    MouseButtonDown,
    MouseButtonUp
};

struct Event
{
    EventType type;

    int32_t width = 0, height = 0;
    float x = 0.0f, y = 0.0f;
    uint8_t axis = 0;
    KeyCode key = KeyCode::A;
    MouseButton mouseButton = MouseButton::Left;
};
}  // namespace Matcha
