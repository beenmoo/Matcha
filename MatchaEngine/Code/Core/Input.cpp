#include "Input.h"

#include <SDL3/SDL.h>
#include <cstring>
#include <utility>

namespace Matcha
{
Input::Input()
{
    m_KeyboardState = SDL_GetKeyboardState(&m_NumKeys);
    m_PrevKeyboardState = new bool[m_NumKeys];
    memcpy(m_PrevKeyboardState, m_KeyboardState, m_NumKeys);

    m_MouseState = SDL_GetMouseState(&m_MouseData.m_MousePositionX, &m_MouseData.m_MousePositionY);
}

Input::~Input()
{
    delete[] m_PrevKeyboardState;
}

void Input::ProcessEvents(const Event& evt)
{
    switch (evt.type)
    {
    case EventType::MouseMoved:
        m_MouseData.m_MouseAxis.x = evt.x;
        m_MouseData.m_MouseAxis.y = evt.y;
        break;
    case EventType::JoystickMoved:
        if (evt.axis == 0)
            m_JoystickAxis.x = evt.x;
        else if (evt.axis == 1)
            m_JoystickAxis.y = evt.x;
        break;
    case EventType::MouseScrolled:
        m_MouseData.m_MouseScrollDelta.x = evt.x;
        m_MouseData.m_MouseScrollDelta.y = evt.y;
        break;
    default:
        break;
    }
}

void Input::Update()
{
    memcpy(m_PrevKeyboardState, m_KeyboardState, m_NumKeys);

    m_MouseData.m_MouseAxis = Vector2Int(0);
    m_MouseData.m_MouseScrollDelta = Vector2Int(0);
    m_PrevMouseState = m_MouseState;
    m_MouseState = SDL_GetMouseState(&m_MouseData.m_MousePositionX, &m_MouseData.m_MousePositionY);
}

bool Input::GetKey(KeyCode code) const
{
    return m_KeyboardState[std::to_underlying(code)];
}

bool Input::GetKeyDown(KeyCode code) const
{
    return !m_PrevKeyboardState[std::to_underlying(code)] && m_KeyboardState[std::to_underlying(code)];
}

bool Input::GetKeyUp(KeyCode code) const
{
    return m_PrevKeyboardState[std::to_underlying(code)] && !m_KeyboardState[std::to_underlying(code)];
}

uint32_t Input::ToMouseButtonMask(MouseButton button)
{
    switch (button)
    {
    case MouseButton::Left:
        return SDL_BUTTON_LMASK;
    case MouseButton::Middle:
        return SDL_BUTTON_MMASK;
    case MouseButton::Right:
        return SDL_BUTTON_RMASK;
    case MouseButton::Back:
        return SDL_BUTTON_X1MASK;
    case MouseButton::Forward:
        return SDL_BUTTON_X2MASK;
    default:
        std::unreachable();
    }
}

bool Input::GetMouseButton(MouseButton button) const
{
    uint32_t mask = ToMouseButtonMask(button);

    return m_MouseState & mask;
}

bool Input::GetMouseButtonDown(MouseButton button) const
{
    uint32_t mask = ToMouseButtonMask(button);

    return !(m_PrevMouseState & mask) && (m_MouseState & mask);
}

bool Input::GetMouseButtonUp(MouseButton button) const
{
    uint32_t mask = ToMouseButtonMask(button);

    return (m_PrevMouseState & mask) && !(m_MouseState & mask);
}

Vector2Int Input::GetAxis(AxisType type) const
{
    switch (type)
    {
    case AxisType::Mouse:
        return m_MouseData.m_MouseAxis;
    case AxisType::Joystick:
        return m_JoystickAxis;
    default:
        break;
    }

    return Vector2Int(0);
}

const Vector2Int& Input::GetMouseScrollDelta() const
{
    return m_MouseData.m_MouseScrollDelta;
}

void Input::SetCursorLockState(CursorLockState state)
{
    m_CursorLockState = state;

    switch (state)
    {
    case CursorLockState::Locked:
        // SDL_SetWindowRelativeMouseMode(true);
        break;
    default:
        // SDL_SetWindowRelativeMouseMode(false);
        break;
    }
}

Input::CursorLockState Input::GetCursorLockState() const
{
    return m_CursorLockState;
}
}  // namespace Matcha