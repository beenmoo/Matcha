#include "CameraController.h"

#include <algorithm>

namespace MatchaEditor
{
void CameraController::Update(Input& input, Time& time, Transform& cameraTransform)
{
    // Look: only while holding the right mouse button. Lock/hide the cursor for the duration so
    // it can't run out of window to move in - SetCursorLockState hides it and, on Qt, warps it
    // back to center each move (SDL's relative mouse mode does the equivalent natively).
    if (input.GetMouseButtonDown(Input::MouseButton::Right))
        input.SetCursorLockState(Input::CursorLockState::Locked);
    else if (input.GetMouseButtonUp(Input::MouseButton::Right))
        input.SetCursorLockState(Input::CursorLockState::None);

    if (input.GetMouseButton(Input::MouseButton::Right))
    {
        constexpr float mouseSensitivity = 0.15f;

        Vector2Int mouseDelta = input.GetAxis(Input::AxisType::Mouse);

        m_Yaw += -static_cast<float>(mouseDelta.x) * mouseSensitivity;
        m_Pitch += -static_cast<float>(mouseDelta.y) * mouseSensitivity;
        m_Pitch = std::clamp(m_Pitch, -89.0f, 89.0f);

        // Rebuilt fresh from the total yaw/pitch every time, not composed incrementally - yaw
        // around world up, pitch around the yawed frame's local right, matching the classic
        // FPS-camera R = Yaw * Pitch order (see the m_Yaw/m_Pitch comment in CameraController.h
        // for why).
        cameraTransform.SetRotation(AngleAxis(Radians(m_Yaw), Vector3(0.0f, 1.0f, 0.0f)) * AngleAxis(Radians(m_Pitch), Vector3(1.0f, 0.0f, 0.0f)));
    }

    // Move: WASD + Space/Ctrl, relative to the camera's own orientation. Gated to the same
    // right-mouse-held "flying" state as look, rather than always-on - matches the Unity/Unreal/
    // Blender convention, and frees WASD/QWERTY-row keys (see ViewportInteraction's W/E/R gizmo
    // mode shortcuts) to mean something else whenever the camera isn't actively being flown.
    if (input.GetMouseButton(Input::MouseButton::Right))
    {
        constexpr float moveSpeed = 2.0f;

        Vector3 movement(0.0f);

        if (input.GetKey(KeyCode::W))
            movement += cameraTransform.GetForward();
        if (input.GetKey(KeyCode::S))
            movement -= cameraTransform.GetForward();
        if (input.GetKey(KeyCode::D))
            movement += cameraTransform.GetRight();
        if (input.GetKey(KeyCode::A))
            movement -= cameraTransform.GetRight();
        if (input.GetKey(KeyCode::SPACE))
            movement += Vector3(0.0f, 1.0f, 0.0f);
        if (input.GetKey(KeyCode::LCTRL))
            movement -= Vector3(0.0f, 1.0f, 0.0f);

        if (movement != Vector3(0.0f))
            cameraTransform.Translate(Normalize(movement) * moveSpeed * time.GetDeltaTime());
    }

    // Zoom: dolly along the camera's own forward direction. GetMouseScrollDelta() is already a
    // discrete "how many notches arrived this frame" quantity (not a held-key rate like WASD
    // above), so it's applied directly rather than scaled by deltaTime - doing that would shrink
    // a single notch to a barely-visible fraction of a unit.
    if (input.GetKey(KeyCode::LCTRL) && input.GetMouseScrollDelta().y != 0.0f)
    {
        constexpr float zoomSpeed = 2.0f;

        cameraTransform.Translate(cameraTransform.GetForward() * input.GetMouseScrollDelta().y * zoomSpeed);
    }

    // Pan: dolly along the camera's own right/left and up/down directions using middle-mouse drag.
    // Locks/hides the cursor for the same reason as look above - so a fast pan can't run the
    // cursor off the window and clip the delta.
    if (input.GetMouseButtonDown(Input::MouseButton::Middle))
        input.SetCursorLockState(Input::CursorLockState::Locked);
    else if (input.GetMouseButtonUp(Input::MouseButton::Middle))
        input.SetCursorLockState(Input::CursorLockState::None);

    if (input.GetMouseButton(Input::MouseButton::Middle))
    {
        constexpr float panSpeed = 2.0f;

        Vector2Int mouseDelta = input.GetAxis(Input::AxisType::Mouse);
        Vector3 right = cameraTransform.GetRight();
        Vector3 up = cameraTransform.GetUp();

        cameraTransform.Translate((-right * static_cast<float>(mouseDelta.x) + up * static_cast<float>(mouseDelta.y)) * panSpeed * time.GetDeltaTime());
    }
}
}  // namespace MatchaEditor
