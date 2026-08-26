#include "Input.h"

#include <SDL3/SDL.h>
#include <cstring>
#include <utility>

namespace Matcha
{
Input::Input()
{
    mKeyboardState = SDL_GetKeyboardState(&mNumKeys);
    mPrevKeyboardState = new bool[mNumKeys];
    memcpy(mPrevKeyboardState, mKeyboardState, mNumKeys);

    mMouseState = SDL_GetMouseState(&mMouseData.mMousePositionX, &mMouseData.mMousePositionY);
}

Input::~Input()
{
    delete[] mPrevKeyboardState;
}

void Input::ProcessEvents(const Event& evt)
{
    switch (evt.type)
    {
    case EventType::MouseMoved:
        mMouseData.mMouseAxis.x = evt.x;
        mMouseData.mMouseAxis.y = evt.y;
        break;
    case EventType::JoystickMoved:
        if (evt.axis == 0)
            mJoystickAxis.x = evt.x;
        else if (evt.axis == 1)
            mJoystickAxis.y = evt.x;
        break;
    case EventType::MouseScrolled:
        mMouseData.mMouseScrollDelta.x = evt.x;
        mMouseData.mMouseScrollDelta.y = evt.y;
        break;
    default:
        break;
    }
}

void Input::Update()
{
    memcpy(mPrevKeyboardState, mKeyboardState, mNumKeys);

    mMouseData.mMouseAxis = Vector2Int(0);
    mMouseData.mMouseScrollDelta = Vector2Int(0);
    mPrevMouseState = mMouseState;
    mMouseState = SDL_GetMouseState(&mMouseData.mMousePositionX, &mMouseData.mMousePositionY);
}

bool Input::GetKey(KeyCode code) const
{
    return mKeyboardState[std::to_underlying(code)];
}

bool Input::GetKeyDown(KeyCode code) const
{
    return !mPrevKeyboardState[std::to_underlying(code)] && mKeyboardState[std::to_underlying(code)];
}

bool Input::GetKeyUp(KeyCode code) const
{
    return mPrevKeyboardState[std::to_underlying(code)] && !mKeyboardState[std::to_underlying(code)];
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

    return mMouseState & mask;
}

bool Input::GetMouseButtonDown(MouseButton button) const
{
    uint32_t mask = ToMouseButtonMask(button);

    return !(mPrevMouseState & mask) && (mMouseState & mask);
}

bool Input::GetMouseButtonUp(MouseButton button) const
{
    uint32_t mask = ToMouseButtonMask(button);

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
        // SDL_SetWindowRelativeMouseMode(true);
        break;
    default:
        // SDL_SetWindowRelativeMouseMode(false);
        break;
    }
}

Input::CursorLockState Input::GetCursorLockState() const
{
    return mCursorLockState;
}
}  // namespace Matcha