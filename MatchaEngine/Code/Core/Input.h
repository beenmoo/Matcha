#pragma once

#include "KeyCodes.h"
#include "Event.h"
#include "Math/Vector.h"

#include <cstdint>

namespace Matcha
{
class Input
{
public:
    enum class AxisType
    {
        Mouse,
        Joystick
    };

    enum class CursorLockState
    {
        None,
        Locked
    };

    enum class MouseButton
    {
        Left,
        Middle,
        Right,
        Back,
        Forward
    };

private:
    struct MouseData
    {
        float m_MousePositionX = 0, m_MousePositionY = 0;
        Vector2Int m_MouseAxis = Vector2Int(0);
        Vector2Int m_MouseScrollDelta = Vector2Int(0);
    };

public:
    Input();
    ~Input();

    void ProcessEvents(const Event& evt);
    void Update();

    [[nodiscard]] bool GetKey(KeyCode code) const;
    [[nodiscard]] bool GetKeyDown(KeyCode code) const;
    [[nodiscard]] bool GetKeyUp(KeyCode code) const;

    [[nodiscard]] bool GetMouseButton(MouseButton button) const;
    [[nodiscard]] bool GetMouseButtonDown(MouseButton button) const;
    [[nodiscard]] bool GetMouseButtonUp(MouseButton button) const;

    [[nodiscard]] Vector2Int GetAxis(AxisType type) const;
    [[nodiscard]] const Vector2Int& GetMouseScrollDelta() const;
    void SetCursorLockState(CursorLockState state);
    [[nodiscard]] CursorLockState GetCursorLockState() const;

private:
    [[nodiscard]] static uint32_t ToMouseButtonMask(MouseButton button);

private:
    const bool* m_KeyboardState;
    bool* m_PrevKeyboardState;
    int m_NumKeys;

    uint32_t m_MouseState;
    uint32_t m_PrevMouseState = 0;
    MouseData m_MouseData;

    Vector2Int m_JoystickAxis = Vector2Int(0);

    CursorLockState m_CursorLockState = CursorLockState::None;
};
}  // namespace Matcha