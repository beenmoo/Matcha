#include "Input.h"

namespace Matcha
{
    Input::Input()
    {
        mKeyboardState = SDL_GetKeyboardState(&mNumKeys);
        mPrevKeyboardState = new uint8_t[mNumKeys];
        memcpy(mPrevKeyboardState, mKeyboardState, mNumKeys);

        mMouseState = SDL_GetMouseState(&mMouseData.mMousePositionX, &mMouseData.mMousePositionY);
    }

    Input::~Input()
    {
        delete[] mPrevKeyboardState;
    }

    void Input::ProcessEvents(const SDL_Event& evt)
    {
        switch (evt.type)
        {
        case SDL_MOUSEMOTION:
            mMouseData.mMouseAxis.x = evt.motion.xrel;
            mMouseData.mMouseAxis.y = evt.motion.yrel;
            break;
        case SDL_JOYAXISMOTION:
            mJoystickAxis.x = evt.motion.xrel;
            mJoystickAxis.y = evt.motion.yrel;
            break;
        case SDL_MOUSEWHEEL:
            mMouseData.mMouseScrollDelta.x = evt.wheel.x;
            mMouseData.mMouseScrollDelta.y = evt.wheel.y;
            break;
        default:
            break;
        }
    }

    void Input::Update()
    {
        memcpy(mPrevKeyboardState, mKeyboardState, mNumKeys);

        mMouseData.mMouseAxis = Vector2(0.0f);
        mMouseData.mMouseScrollDelta = Vector2(0.0f);
        mPrevMouseState = mMouseState;
        mMouseState = SDL_GetMouseState(&mMouseData.mMousePositionX, &mMouseData.mMousePositionY);
    }

    bool Input::GetKey(KeyCode code) const
    {
        return mKeyboardState[(uint8_t)code];
    }

    bool Input::GetKeyDown(KeyCode code) const
    {
        return !mPrevKeyboardState[(uint8_t)code] && mKeyboardState[(uint8_t)code];
    }

    bool Input::GetKeyUp(KeyCode code) const
    {
        return mPrevKeyboardState[(uint8_t)code] && !mKeyboardState[(uint8_t)code];
    }

    bool Input::GetMouseButton(MouseButton button) const
    {
        uint32_t mask = 0;

        switch (button)
        {
        case MouseButton::Left:
            mask = SDL_BUTTON_LMASK;
            break;
        case MouseButton::Middle:
            mask = SDL_BUTTON_MMASK;
            break;
        case MouseButton::Right:
            mask = SDL_BUTTON_RMASK;
            break;
        case MouseButton::Back:
            mask = SDL_BUTTON_X1MASK;
            break;
        case MouseButton::Forward:
            mask = SDL_BUTTON_X2MASK;
            break;
        default:
            break;
        }

        return mMouseState & mask;
    }

    bool Input::GetMouseButtonDown(MouseButton button) const
    {
        uint32_t mask = 0;

        switch (button)
        {
        case MouseButton::Left:
            mask = SDL_BUTTON_LMASK;
            break;
        case MouseButton::Middle:
            mask = SDL_BUTTON_MMASK;
            break;
        case MouseButton::Right:
            mask = SDL_BUTTON_RMASK;
            break;
        case MouseButton::Back:
            mask = SDL_BUTTON_X1MASK;
            break;
        case MouseButton::Forward:
            mask = SDL_BUTTON_X2MASK;
            break;
        default:
            break;
        }

        return !(mPrevMouseState & mask) && (mMouseState & mask);
    }

    bool Input::GetMouseButtonUp(MouseButton button) const
    {
        uint32_t mask = 0;

        switch (button)
        {
        case MouseButton::Left:
            mask = SDL_BUTTON_LMASK;
            break;
        case MouseButton::Middle:
            mask = SDL_BUTTON_MMASK;
            break;
        case MouseButton::Right:
            mask = SDL_BUTTON_RMASK;
            break;
        case MouseButton::Back:
            mask = SDL_BUTTON_X1MASK;
            break;
        case MouseButton::Forward:
            mask = SDL_BUTTON_X2MASK;
            break;
        default:
            break;
        }

        return (mPrevMouseState & mask) && !(mMouseState & mask);
    }

    Vector2Int Input::GetAxis(AxisType type) const
    {
        switch (type)
        {
        case AxisType::Mouse:
            return mMouseData.mMouseAxis;
        case AxisType::Joystick:
            return mJoystickAxis;
        default:
            break;
        }

        return Vector2Int(0);
    }

    const Vector2Int& Input::GetMouseScrollDelta() const
    {
        return mMouseData.mMouseScrollDelta;
    }

    void Input::SetCursorLockState(CursorLockState state)
    {
        mCursorLockState = state;

        switch (state)
        {
        case CursorLockState::Locked:
            SDL_SetRelativeMouseMode(SDL_TRUE);
            break;
        default:
            SDL_SetRelativeMouseMode(SDL_FALSE);
            break;
        }
    }
    
    Input::CursorLockState Input::GetCursorLockState() const
    {
        return mCursorLockState;
    }
}