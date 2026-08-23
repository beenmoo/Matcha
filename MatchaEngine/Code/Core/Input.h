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
        float mMousePositionX = 0, mMousePositionY = 0;
        Vector2 mMouseAxis = Vector2(0);
        Vector2 mMouseScrollDelta = Vector2(0);
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
    const bool* mKeyboardState;
    bool* mPrevKeyboardState;
    int mNumKeys;

    uint32_t mMouseState;
    uint32_t mPrevMouseState = 0;
    MouseData mMouseData;

    Vector2Int mJoystickAxis = Vector2(0.0f);

    CursorLockState mCursorLockState = CursorLockState::None;
};
}  // namespace Matcha