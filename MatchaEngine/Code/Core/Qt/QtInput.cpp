#include "QtInput.h"
#include "QtViewportWidget.h"

#include <utility>

namespace Matcha
{
void QtInput::ProcessEvents(const Event& evt)
{
    ApplyAxisEvent(evt, m_MouseAxis, m_JoystickAxis, m_MouseScrollDelta);
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
    return WentDown(m_PrevKeyboardState[std::to_underlying(code)], m_KeyboardState[std::to_underlying(code)]);
}

bool QtInput::GetKeyUp(KeyCode code) const
{
    return WentUp(m_PrevKeyboardState[std::to_underlying(code)], m_KeyboardState[std::to_underlying(code)]);
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

    return WentDown(m_PrevMouseButtonState[index], m_MouseButtonState[index]);
}

bool QtInput::GetMouseButtonUp(MouseButton button) const
{
    size_t index = ToIndex(button);

    return WentUp(m_PrevMouseButtonState[index], m_MouseButtonState[index]);
}

Vector2Int QtInput::GetAxis(AxisType type) const
{
    return SelectAxis(type, m_MouseAxis, m_JoystickAxis);
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
    m_PendingKeyboardState[std::to_underlying(code)] = true;
}

void QtInput::PushKeyUp(KeyCode code)
{
    m_PendingKeyboardState[std::to_underlying(code)] = false;
}

void QtInput::PushMouseButtonDown(MouseButton button)
{
    m_PendingMouseButtonState[ToIndex(button)] = true;
}

void QtInput::PushMouseButtonUp(MouseButton button)
{
    m_PendingMouseButtonState[ToIndex(button)] = false;
}

void QtInput::ResetKeyboard()
{
    m_PendingKeyboardState.fill(false);
}

void QtInput::ApplyPendingInput()
{
    // Qt calls PushKeyDown()/PushMouseButtonDown() etc. the instant it delivers the underlying
    // event - asynchronously, whenever that happens to be relative to the Tick() cycle, not
    // synchronized to any particular point in it. If those wrote straight into
    // m_KeyboardState/m_MouseButtonState (the previous behavior), a button held across multiple
    // frames would already show as "current" by the time Update() ran its prev = current shift,
    // every single frame - so prev and current were always equal by the time GetKeyDown()/
    // GetMouseButtonDown() read them, and the down/up edge could never be observed. Landing
    // pushes in m_Pending*State instead, then only copying pending -> current here (called from
    // QtWindow::PumpEvents(), right after Update()'s shift), reproduces SDL's ordering: prev
    // captures the state as of before this frame's events, current captures state as of after.
    m_KeyboardState = m_PendingKeyboardState;
    m_MouseButtonState = m_PendingMouseButtonState;
}

void QtInput::SetViewportWidget(QtViewportWidget* viewportWidget)
{
    m_ViewportWidget = viewportWidget;
}
}  // namespace Matcha
