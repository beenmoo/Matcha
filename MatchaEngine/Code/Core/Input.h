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

    // Alias, not a redeclaration: Event.h defines this at namespace scope so Event can carry a
    // MouseButton payload without depending on Input.h (which depends on Event.h) - every existing
    // Input::MouseButton::Left-style call site keeps compiling unchanged.
    using MouseButton = Matcha::MouseButton;

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

    // Vector2, not Vector2Int like GetAxis() above: a wheel event's delta (Qt's angleDelta/120,
    // SDL's wheel.x/y) is genuinely fractional on many mice/trackpads (high-resolution/smooth-
    // scroll wheels report sub-notch deltas) - accumulating that into an int truncates every
    // single sub-1.0 event straight to zero, silently swallowing all scroll input on such devices.
    [[nodiscard]] virtual const Vector2& GetMouseScrollDelta() const = 0;
    virtual void SetCursorLockState(CursorLockState state) = 0;
    [[nodiscard]] virtual CursorLockState GetCursorLockState() const = 0;

    [[nodiscard]] static std::unique_ptr<Input> Create(WindowBackend backend);

protected:
    // Every backend translates Event's mouse/joystick/scroll payload into these three the same
    // way - shared here rather than duplicated per backend's ProcessEvents().
    static void ApplyAxisEvent(const Event& evt, Vector2Int& mouseAxis, Vector2Int& joystickAxis, Vector2& mouseScrollDelta);

    // Same prev/current edge-detection formula backs GetKeyDown/GetKeyUp and
    // GetMouseButtonDown/GetMouseButtonUp in every backend.
    [[nodiscard]] static bool WentDown(bool previous, bool current)
    {
        return !previous && current;
    }

    [[nodiscard]] static bool WentUp(bool previous, bool current)
    {
        return previous && !current;
    }

    // AxisType -> the matching axis vector - shared GetAxis() body for every backend.
    [[nodiscard]] static Vector2Int SelectAxis(AxisType type, const Vector2Int& mouseAxis, const Vector2Int& joystickAxis);
};
}  // namespace Matcha
