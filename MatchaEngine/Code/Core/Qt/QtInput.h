#pragma once

#include "Core/Input.h"

#include <array>
#include <cstddef>

namespace Matcha
{
class QtViewportWidget;

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
    // the way mouse-axis/scroll do. Only ever touch m_Pending*State (see ApplyPendingInput()).
    void PushKeyDown(KeyCode code);
    void PushKeyUp(KeyCode code);
    void PushMouseButtonDown(MouseButton button);
    void PushMouseButtonUp(MouseButton button);

    // Copies m_Pending*State (live - Push*() writes land here the instant Qt delivers an event,
    // whenever that is) into m_KeyboardState/m_MouseButtonState (settled - only ever changes
    // here, once per frame). Called from QtWindow::PumpEvents(), which Application::Tick() invokes
    // right after Input::Update() shifts current into prev and before Update()/OnUpdate() read
    // GetKeyDown()/GetMouseButtonDown() - see the .cpp for why applying pushes immediately instead
    // (the previous behavior) made those edges undetectable.
    void ApplyPendingInput();

    // Wired up by QtWindow at construction time, since SetCursorLockState needs the viewport
    // widget to actually hide/warp the cursor (Qt has no built-in relative mouse mode).
    void SetViewportWidget(QtViewportWidget* viewportWidget);

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
    Vector2Int m_MouseScrollDelta = Vector2Int(0);
    Vector2Int m_JoystickAxis = Vector2Int(0);

    CursorLockState m_CursorLockState = CursorLockState::None;
    QtViewportWidget* m_ViewportWidget = nullptr;
};
}  // namespace Matcha
