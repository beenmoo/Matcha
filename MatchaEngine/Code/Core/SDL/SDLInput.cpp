#include "SDLInput.h"

#include <SDL3/SDL.h>
#include <utility>

namespace Matcha
{
void SDLInput::ProcessEvents(const Event& evt)
{
    ApplyAxisEvent(evt, m_MouseAxis, m_JoystickAxis, m_MouseScrollDelta);

    switch (evt.type)
    {
    case EventType::KeyDown:
        m_PendingKeyboardState[std::to_underlying(evt.key)] = true;
        break;
    case EventType::KeyUp:
        m_PendingKeyboardState[std::to_underlying(evt.key)] = false;
        break;
    case EventType::MouseButtonDown:
        m_PendingMouseButtonState[ToIndex(evt.mouseButton)] = true;
        break;
    case EventType::MouseButtonUp:
        m_PendingMouseButtonState[ToIndex(evt.mouseButton)] = false;
        break;
    case EventType::WindowFocusLost:
        // Once a window loses focus, the OS stops delivering key-up events to it - a key held
        // down (e.g. W while moving the camera) that gets released while focus is elsewhere
        // (alt-tab, clicking another window) would otherwise keep reporting as down indefinitely,
        // even after focus returns. Clearing the pending buffer (not m_KeyboardState directly)
        // takes effect through the same ApplyPendingInput() path as every other key event, one
        // frame later - consistent with how every other push lands.
        m_PendingKeyboardState.fill(false);
        break;
    default:
        break;
    }
}

void SDLInput::Update()
{
    m_PrevKeyboardState = m_KeyboardState;
    m_PrevMouseButtonState = m_MouseButtonState;

    m_MouseAxis = Vector2Int(0);
    m_MouseScrollDelta = Vector2(0.0f);
}

bool SDLInput::GetKey(KeyCode code) const
{
    return m_KeyboardState[std::to_underlying(code)];
}

bool SDLInput::GetKeyDown(KeyCode code) const
{
    return WentDown(m_PrevKeyboardState[std::to_underlying(code)], m_KeyboardState[std::to_underlying(code)]);
}

bool SDLInput::GetKeyUp(KeyCode code) const
{
    return WentUp(m_PrevKeyboardState[std::to_underlying(code)], m_KeyboardState[std::to_underlying(code)]);
}

size_t SDLInput::ToIndex(MouseButton button)
{
    return static_cast<size_t>(button);
}

bool SDLInput::GetMouseButton(MouseButton button) const
{
    return m_MouseButtonState[ToIndex(button)];
}

bool SDLInput::GetMouseButtonDown(MouseButton button) const
{
    size_t index = ToIndex(button);

    return WentDown(m_PrevMouseButtonState[index], m_MouseButtonState[index]);
}

bool SDLInput::GetMouseButtonUp(MouseButton button) const
{
    size_t index = ToIndex(button);

    return WentUp(m_PrevMouseButtonState[index], m_MouseButtonState[index]);
}

Vector2Int SDLInput::GetAxis(AxisType type) const
{
    return SelectAxis(type, m_MouseAxis, m_JoystickAxis);
}

const Vector2& SDLInput::GetMouseScrollDelta() const
{
    return m_MouseScrollDelta;
}

void SDLInput::SetCursorLockState(CursorLockState state)
{
    m_CursorLockState = state;

    if (m_ExternalCursorLockCallback)
        m_ExternalCursorLockCallback(state == CursorLockState::Locked);
    else if (m_NativeWindow)
        SDL_SetWindowRelativeMouseMode(m_NativeWindow, state == CursorLockState::Locked);
}

Input::CursorLockState SDLInput::GetCursorLockState() const
{
    return m_CursorLockState;
}

void SDLInput::SetNativeWindow(SDL_Window* window)
{
    m_NativeWindow = window;
}

void SDLInput::SetExternalCursorLockCallback(std::function<void(bool)> callback)
{
    m_ExternalCursorLockCallback = std::move(callback);
}

void SDLInput::ApplyPendingInput()
{
    m_KeyboardState = m_PendingKeyboardState;
    m_MouseButtonState = m_PendingMouseButtonState;
}
}  // namespace Matcha
