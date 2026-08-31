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

protected:
    // Every backend translates Event's mouse/joystick/scroll payload into these three the same
    // way - shared here rather than duplicated per backend's ProcessEvents().
    static void ApplyAxisEvent(const Event& evt, Vector2Int& mouseAxis, Vector2Int& joystickAxis, Vector2Int& mouseScrollDelta);

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
