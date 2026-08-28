#pragma once

#include <Matcha.h>

namespace Matcha
{
class CameraController : public ScriptableEntity
{
protected:
    void OnUpdate() override;

private:
    // Accumulated separately (degrees) and rebuilt into the camera's rotation from scratch each
    // time, rather than incrementally composing World/Self Rotate() calls onto the running
    // quaternion frame after frame - the latter accumulates roll drift over time, since applying
    // a new world-space yaw on top of a rotation that already has pitch baked into it isn't the
    // same as recomputing yaw*pitch fresh from the total angles.
    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;
};
}  // namespace Matcha
