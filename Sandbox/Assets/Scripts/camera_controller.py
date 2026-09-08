import matcha_engine

# Port of Sandbox/Code/Core/CameraController. Free-fly movement (WASD + Space/Ctrl) and
# mouse-look (held right mouse button), relative to the camera entity's own orientation.

MOUSE_SENSITIVITY = 0.15
MOVE_SPEED = 2.0


class CameraController:
    # See RotationComponent's own copy of this comment for why these are bare annotations.
    entity: matcha_engine.Entity
    input: matcha_engine.Input
    time: matcha_engine.Time

    def __init__(self):
        # Accumulated separately (degrees) and rebuilt into the camera's rotation from scratch
        # each time, rather than incrementally composing rotations onto the running quaternion
        # frame after frame - see the same field in the original CameraController.h for why
        # (composing accumulates roll drift; recomputing yaw*pitch fresh from the totals doesn't).
        self.yaw = 0.0
        self.pitch = 0.0

    def on_update(self):
        transform = self.entity.get_transform()
        input = self.input

        # Look: only while holding the right mouse button. Lock/hide the cursor for the duration.
        if input.get_mouse_button_down(matcha_engine.MouseButton.RIGHT):
            input.set_cursor_lock_state(matcha_engine.CursorLockState.LOCKED)
        elif input.get_mouse_button_up(matcha_engine.MouseButton.RIGHT):
            input.set_cursor_lock_state(matcha_engine.CursorLockState.NONE)

        if input.get_mouse_button(matcha_engine.MouseButton.RIGHT):
            mouse_delta = input.get_axis(matcha_engine.AxisType.MOUSE)

            self.yaw += -float(mouse_delta.x) * MOUSE_SENSITIVITY
            self.pitch += -float(mouse_delta.y) * MOUSE_SENSITIVITY
            self.pitch = max(-89.0, min(89.0, self.pitch))

            # Yaw around world up, pitch around the yawed frame's local right - classic FPS-camera
            # R = Yaw * Pitch order.
            yaw_rotation = matcha_engine.angle_axis(matcha_engine.radians(self.yaw), matcha_engine.Vector3(0.0, 1.0, 0.0))
            pitch_rotation = matcha_engine.angle_axis(matcha_engine.radians(self.pitch), matcha_engine.Vector3(1.0, 0.0, 0.0))
            transform.set_rotation(yaw_rotation * pitch_rotation)

        # Move: WASD + Space/Ctrl, relative to the camera's own orientation.
        movement = matcha_engine.Vector3(0.0)

        if input.get_key(matcha_engine.KeyCode.W):
            movement = movement + transform.get_forward()
        if input.get_key(matcha_engine.KeyCode.S):
            movement = movement - transform.get_forward()
        if input.get_key(matcha_engine.KeyCode.D):
            movement = movement + transform.get_right()
        if input.get_key(matcha_engine.KeyCode.A):
            movement = movement - transform.get_right()
        if input.get_key(matcha_engine.KeyCode.SPACE):
            movement = movement + matcha_engine.Vector3(0.0, 1.0, 0.0)
        if input.get_key(matcha_engine.KeyCode.LCTRL):
            movement = movement - matcha_engine.Vector3(0.0, 1.0, 0.0)

        if movement != matcha_engine.Vector3(0.0):
            delta_time = self.time.get_delta_time()
            transform.translate(matcha_engine.normalize(movement) * MOVE_SPEED * delta_time)
