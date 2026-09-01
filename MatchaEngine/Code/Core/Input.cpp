#include "Input.h"
#include "Assert.h"
#include "SDL/SDLInput.h"

#ifdef MT_ENABLE_QT_BACKEND
#include "Qt/QtInput.h"
#endif

namespace Matcha
{
void Input::ApplyAxisEvent(const Event& evt, Vector2Int& mouseAxis, Vector2Int& joystickAxis, Vector2Int& mouseScrollDelta)
{
    switch (evt.type)
    {
    case EventType::MouseMoved:
        // Accumulate, not overwrite: SDL delivers one MouseMoved event per raw motion report, and
        // PumpEvents() drains all of them each frame - under WSL2 relative mode in particular,
        // several small reports commonly arrive within a single frame, so summing is the only way
        // to capture the frame's total movement instead of just its last report.
        mouseAxis.x += evt.x;
        mouseAxis.y += evt.y;
        break;
    case EventType::JoystickMoved:
        // Overwrite, not accumulate: unlike mouse motion, an axis-motion event reports the axis's
        // current absolute position, not a delta since the last event - summing would runaway.
        if (evt.axis == 0)
            joystickAxis.x = evt.x;
        else if (evt.axis == 1)
            joystickAxis.y = evt.x;
        break;
    case EventType::MouseScrolled:
        // Same accumulate-not-overwrite reasoning as MouseMoved: each wheel event reports a
        // per-event scroll delta, so multiple events in one frame need to sum.
        mouseScrollDelta.x += evt.x;
        mouseScrollDelta.y += evt.y;
        break;
    default:
        break;
    }
}

Vector2Int Input::SelectAxis(AxisType type, const Vector2Int& mouseAxis, const Vector2Int& joystickAxis)
{
    switch (type)
    {
    case AxisType::Mouse:
        return mouseAxis;
    case AxisType::Joystick:
        return joystickAxis;
    default:
        break;
    }

    return Vector2Int(0);
}

std::unique_ptr<Input> Input::Create(WindowBackend backend)
{
    switch (backend)
    {
    case WindowBackend::SDL:
        return std::make_unique<SDLInput>();
#ifdef MT_ENABLE_QT_BACKEND
    case WindowBackend::Qt:
        return std::make_unique<QtInput>();
#endif
    default:
        MT_ASSERT(false, "Input backend not supported (was MatchaEngine built with BUILD_QT_BACKEND?)");
        return nullptr;
    }
}
}  // namespace Matcha
