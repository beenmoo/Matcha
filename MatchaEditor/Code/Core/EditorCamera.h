#pragma once

#include "CameraController.h"

#include <Matcha.h>

namespace MatchaEditor
{
class EditorCamera
{
public:
    EditorCamera() = default;

    void Update(EngineContext& context);

    // Recomputed from the viewport's own size, not CameraComponent::aspectRatio on some scene
    // entity - this camera isn't in any Scene, so nothing would ever set that for it.
    void SetAspectRatio(float aspectRatio);

    [[nodiscard]] Matrix4 GetViewProjection() const;
    [[nodiscard]] const Vector3& GetPosition() const;

private:
    Transform m_Transform;
    CameraController m_CameraController;
    CameraComponent m_CameraComponent;
};
}  // namespace MatchaEditor