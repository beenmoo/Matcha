#include "QtInput.h"
#include "QtViewportWidget.h"

#include <utility>

namespace Matcha
{
void QtInput::ProcessEvents(const Event& evt)
{
    switch (evt.type)
    {
    case EventType::MouseMoved:
        m_MouseAxis.x = evt.x;
        m_MouseAxis.y = evt.y;
        break;
    case EventType::JoystickMoved:
        if (evt.axis == 0)
            m_JoystickAxis.x = evt.x;
        else if (evt.axis == 1)
            m_JoystickAxis.y = evt.x;
        break;
    case EventType::MouseScrolled:
        m_MouseScrollDelta.x = evt.x;
        m_MouseScrollDelta.y = evt.y;
        break;
    default:
        break;
    }
}

void QtInput::Update()
{
    m_PrevKeyboardState = m_KeyboardState;
    m_PrevMouseButtonState = m_MouseButtonState;

    m_MouseAxis = Vector2Int(0);
    m_MouseScrollDelta = Vector2Int(0);
}

bool QtInput::GetKey(KeyCode code) const
{
    return m_KeyboardState[std::to_underlying(code)];
}

bool QtInput::GetKeyDown(KeyCode code) const
{
    return !m_PrevKeyboardState[std::to_underlying(code)] && m_KeyboardState[std::to_underlying(code)];
}

bool QtInput::GetKeyUp(KeyCode code) const
{
    return m_PrevKeyboardState[std::to_underlying(code)] && !m_KeyboardState[std::to_underlying(code)];
}

size_t QtInput::ToIndex(MouseButton button)
{
    return static_cast<size_t>(button);
}

bool QtInput::GetMouseButton(MouseButton button) const
{
    return m_MouseButtonState[ToIndex(button)];
}

bool QtInput::GetMouseButtonDown(MouseButton button) const
{
    size_t index = ToIndex(button);

    return !m_PrevMouseButtonState[index] && m_MouseButtonState[index];
}

bool QtInput::GetMouseButtonUp(MouseButton button) const
{
    size_t index = ToIndex(button);

    return m_PrevMouseButtonState[index] && !m_MouseButtonState[index];
}

Vector2Int QtInput::GetAxis(AxisType type) const
{
    switch (type)
    {
    case AxisType::Mouse:
        return m_MouseAxis;
    case AxisType::Joystick:
        return m_JoystickAxis;
    default:
        break;
    }

    return Vector2Int(0);
}

const Vector2Int& QtInput::GetMouseScrollDelta() const
{
    return m_MouseScrollDelta;
}

void QtInput::SetCursorLockState(CursorLockState state)
{
    m_CursorLockState = state;

    if (m_ViewportWidget)
        m_ViewportWidget->SetCursorLocked(state == CursorLockState::Locked);
}

Input::CursorLockState QtInput::GetCursorLockState() const
{
    return m_CursorLockState;
}

void QtInput::PushKeyDown(KeyCode code)
{
    m_KeyboardState[std::to_underlying(code)] = true;
}

void QtInput::PushKeyUp(KeyCode code)
{
    m_KeyboardState[std::to_underlying(code)] = false;
}

void QtInput::PushMouseButtonDown(MouseButton button)
{
    m_MouseButtonState[ToIndex(button)] = true;
}

void QtInput::PushMouseButtonUp(MouseButton button)
{
    m_MouseButtonState[ToIndex(button)] = false;
}

void QtInput::SetViewportWidget(QtViewportWidget* viewportWidget)
{
    m_ViewportWidget = viewportWidget;
}
}  // namespace Matcha
