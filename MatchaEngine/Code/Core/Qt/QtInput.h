#pragma once

#include "Core/Input.h"

#include <array>
#include <cstddef>

namespace Matcha
{
class QtInput final : public Input
{
public:
    QtInput() = default;
    ~QtInput() override = default;

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

    // Qt-specific: only QtViewportWidget calls these, from its own overridden key/mouse event
    // handlers. Qt delivers input via callbacks rather than the global-state polling SDLInput
    // uses, and Event (Core/Event.h) carries no key/button data to route through ProcessEvents
    // the way mouse-axis/scroll do.
    void PushKeyDown(KeyCode code);
    void PushKeyUp(KeyCode code);
    void PushMouseButtonDown(MouseButton button);
    void PushMouseButtonUp(MouseButton button);

private:
    [[nodiscard]] static size_t ToIndex(MouseButton button);

private:
    std::array<bool, static_cast<size_t>(KeyCode::NUM_SCANCODES)> m_KeyboardState{};
    std::array<bool, static_cast<size_t>(KeyCode::NUM_SCANCODES)> m_PrevKeyboardState{};

    static constexpr size_t kMouseButtonCount = 5;
    std::array<bool, kMouseButtonCount> m_MouseButtonState{};
    std::array<bool, kMouseButtonCount> m_PrevMouseButtonState{};

    Vector2Int m_MouseAxis = Vector2Int(0);
    Vector2Int m_MouseScrollDelta = Vector2Int(0);
    Vector2Int m_JoystickAxis = Vector2Int(0);

    CursorLockState m_CursorLockState = CursorLockState::None;
};
}  // namespace Matcha
