#pragma once

#include "KeyCodes.h"
#include "Event.h"
#include "Math/Vector.h"
#include "Window.h"

#include <cstdint>
#include <memory>

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
    virtual ~Input() = default;

    virtual void ProcessEvents(const Event& evt) = 0;
    virtual void Update() = 0;

    [[nodiscard]] virtual bool GetKey(KeyCode code) const = 0;
    [[nodiscard]] virtual bool GetKeyDown(KeyCode code) const = 0;
    [[nodiscard]] virtual bool GetKeyUp(KeyCode code) const = 0;

    [[nodiscard]] virtual bool GetMouseButton(MouseButton button) const = 0;
    [[nodiscard]] virtual bool GetMouseButtonDown(MouseButton button) const = 0;
    [[nodiscard]] virtual bool GetMouseButtonUp(MouseButton button) const = 0;

    [[nodiscard]] virtual Vector2Int GetAxis(AxisType type) const = 0;
    [[nodiscard]] virtual const Vector2Int& GetMouseScrollDelta() const = 0;
    virtual void SetCursorLockState(CursorLockState state) = 0;
    [[nodiscard]] virtual CursorLockState GetCursorLockState() const = 0;

    [[nodiscard]] static std::unique_ptr<Input> Create(WindowBackend backend);
};
}  // namespace Matcha
