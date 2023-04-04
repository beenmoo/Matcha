#pragma once

#include "KeyCodes.h"
#include "Math/Vector.h"

#include <sdl/SDL.h>

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
        struct MouseData
        {
            int mMousePositionX = 0, mMousePositionY = 0;
            Vector2Int mMouseAxis = Vector2(0);
            Vector2Int mMouseScrollDelta = Vector2(0);
        };

    private:
        const uint8_t* mKeyboardState;
        uint8_t* mPrevKeyboardState;
        int mNumKeys;

        uint32_t mMouseState;
        uint32_t mPrevMouseState = 0;
        MouseData mMouseData;

        Vector2Int mJoystickAxis = Vector2(0.0f);

        CursorLockState mCursorLockState = CursorLockState::None;
    };
}