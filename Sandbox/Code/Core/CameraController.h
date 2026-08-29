#pragma once

#include <Matcha.h>

class CameraController : public ScriptableEntity
{
protected:
    void OnCreate() override;
    void OnUpdate() override;

private:
    // Accumulated separately (degrees) and rebuilt into the camera's rotation from scratch each
    // time, rather than incrementally composing World/Self Rotate() calls onto the running
    // quaternion frame after frame - the latter accumulates roll drift over time, since applying
    // a new world-space yaw on top of a rotation that already has pitch baked into it isn't the
    // same as recomputing yaw*pitch fresh from the total angles.
    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    // Created in OnCreate() and synced to the camera's transform every OnUpdate() - there's no
    // parent/child relationship, just a plain entity whose transform gets overwritten each frame,
    // so it always matches wherever the camera currently is/looks.
    Entity m_Flashlight;
};
