#include "CameraController.h"

#include <algorithm>

void CameraController::OnUpdate()
{
    Input& input = GetContext().GetInput();
    Transform& cameraTransform = GetComponent<TransformComponent>().transform;

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

    // Move: WASD + Space/Ctrl, relative to the camera's own orientation.
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
        cameraTransform.Translate(Normalize(movement) * moveSpeed * GetContext().GetTime().GetDeltaTime());
}
