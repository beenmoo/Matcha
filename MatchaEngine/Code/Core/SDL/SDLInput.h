#pragma once

#include "Core/Input.h"

struct SDL_Window;

namespace Matcha
{
class SDLInput final : public Input
{
private:
    struct MouseData
    {
        float m_MousePositionX = 0, m_MousePositionY = 0;
        Vector2Int m_MouseAxis = Vector2Int(0);
        Vector2Int m_MouseScrollDelta = Vector2Int(0);
    };

public:
    SDLInput();
    ~SDLInput() override;

    void ProcessEvents(const Event& evt) override;
    void Update() override;

    [[nodiscard]] bool GetKey(KeyCode code) const override;
    [[nodiscard]] bool GetKeyDown(KeyCode code) const override;
    [[nodiscard]] bool GetKeyUp(KeyCode code) const override;

    [[nodiscard]] bool GetMouseButton(MouseButton button) const override;
    [[nodiscard]] bool GetMouseButtonDown(MouseButton button) const override;
    [[nodiscard]] bool GetMouseButtonUp(MouseButton button) const override;

    [[nodiscard]] Vector2Int GetAxis(AxisType type) const override;
    [[nodiscard]] const Vector2Int& GetMouseScrollDelta() const override;
    void SetCursorLockState(CursorLockState state) override;
    [[nodiscard]] CursorLockState GetCursorLockState() const override;

    // SDL-specific: wired up by SDLWindow at construction time, since SetCursorLockState needs
    // the native window to actually call SDL_SetWindowRelativeMouseMode on.
    void SetNativeWindow(SDL_Window* window);

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
    SDL_Window* m_NativeWindow = nullptr;
};
}  // namespace Matcha
