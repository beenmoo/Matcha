#pragma once

#include "KeyCodes.h"
#include "Math/Vector.h"

#include <SDL3/SDL.h>

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

        void ProcessEvents(const SDL_Event& evt);
        void Update();

        bool GetKey(KeyCode code) const;
        bool GetKeyDown(KeyCode code) const;
        bool GetKeyUp(KeyCode code) const;

        bool GetMouseButton(MouseButton button) const;
        bool GetMouseButtonDown(MouseButton button) const;
        bool GetMouseButtonUp(MouseButton button) const;

        Vector2Int GetAxis(AxisType type) const;
        const Vector2Int& GetMouseScrollDelta() const;
        void SetCursorLockState(CursorLockState state);
        CursorLockState GetCursorLockState() const;

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
}