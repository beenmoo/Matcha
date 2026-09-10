#pragma once

#include "Core/Input.h"

#include <array>
#include <cstddef>
#include <functional>

struct SDL_Window;

namespace Matcha
{
class SDLInput final : public Input
{
public:
    SDLInput() = default;
    ~SDLInput() override = default;

    void ProcessEvents(const Event& evt) override;
    void Update() override;

    [[nodiscard]] bool GetKey(KeyCode code) const override;
    [[nodiscard]] bool GetKeyDown(KeyCode code) const override;
    [[nodiscard]] bool GetKeyUp(KeyCode code) const override;

    [[nodiscard]] bool GetMouseButton(MouseButton button) const override;
    [[nodiscard]] bool GetMouseButtonDown(MouseButton button) const override;
    [[nodiscard]] bool GetMouseButtonUp(MouseButton button) const override;

    [[nodiscard]] Vector2Int GetAxis(AxisType type) const override;
    [[nodiscard]] const Vector2& GetMouseScrollDelta() const override;
    void SetCursorLockState(CursorLockState state) override;
    [[nodiscard]] CursorLockState GetCursorLockState() const override;

    // SDL-specific: wired up by SDLWindow at construction time, since SetCursorLockState needs
    // the native window to actually call SDL_SetWindowRelativeMouseMode on.
    void SetNativeWindow(SDL_Window* window);

    // MatchaEditor-only: SDL's own relative-mouse-mode is meaningless on the hidden, unfocused
    // window a headless Application creates (see WindowSpecification::m_Headless) - real RMB
    // fly-look input arrives via Qt's EngineViewportWidget instead, which has no built-in relative
    // mouse mode of its own (see its warp-to-center PollCursorLock(), ported from the old
    // QtViewportWidget/QtInput pairing). Registering this redirects SetCursorLockState() to that
    // widget instead of SDL_SetWindowRelativeMouseMode.
    void SetExternalCursorLockCallback(std::function<void(bool locked)> callback);

    // Called by SDLWindow::PumpEvents(), right after draining SDL_PollEvent (real hardware events)
    // and after any external host (MatchaEditor's EngineViewportWidget, via
    // SDLWindow::DispatchExternalEvent) has pushed its own translated Qt events for this frame -
    // copies m_Pending*State (live - ProcessEvents() writes land here the instant an event is
    // seen, whenever that is) into m_KeyboardState/m_MouseButtonState (settled - only ever changes
    // here, once per frame). See QtInput::ApplyPendingInput()'s own comment (the pattern this
    // generalizes): landing pushes in a pending buffer first, rather than writing straight into
    // current state, is what makes a down/up edge observable at all when events can arrive at
    // arbitrary times relative to Application::Tick()'s Input::Update() reset.
    void ApplyPendingInput();

private:
    [[nodiscard]] static size_t ToIndex(MouseButton button);

private:
    std::array<bool, static_cast<size_t>(KeyCode::NUM_SCANCODES)> m_PendingKeyboardState{};
    std::array<bool, static_cast<size_t>(KeyCode::NUM_SCANCODES)> m_KeyboardState{};
    std::array<bool, static_cast<size_t>(KeyCode::NUM_SCANCODES)> m_PrevKeyboardState{};

    static constexpr size_t kMouseButtonCount = 5;
    std::array<bool, kMouseButtonCount> m_PendingMouseButtonState{};
    std::array<bool, kMouseButtonCount> m_MouseButtonState{};
    std::array<bool, kMouseButtonCount> m_PrevMouseButtonState{};

    Vector2Int m_MouseAxis = Vector2Int(0);
    Vector2 m_MouseScrollDelta = Vector2(0.0f);
    Vector2Int m_JoystickAxis = Vector2Int(0);

    CursorLockState m_CursorLockState = CursorLockState::None;
    SDL_Window* m_NativeWindow = nullptr;
    std::function<void(bool)> m_ExternalCursorLockCallback;
};
}  // namespace Matcha
