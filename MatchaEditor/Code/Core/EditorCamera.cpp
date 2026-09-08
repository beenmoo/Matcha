#include "EditorCamera.h"

namespace MatchaEditor
{
void EditorCamera::Update(Input& input, Time& time)
{
    m_CameraController.Update(input, time, m_Transform);
}

void EditorCamera::SetAspectRatio(float aspectRatio)
{
    m_CameraComponent.aspectRatio = aspectRatio;
}

Matrix4 EditorCamera::GetViewProjection() const
{
    Matrix4 projection = Perspective(m_CameraComponent.perspectiveFOV, m_CameraComponent.aspectRatio,
                                      m_CameraComponent.perspectiveNear, m_CameraComponent.perspectiveFar);

    return projection * Inverse(m_Transform.GetLocalMatrix());
}

const Vector3& EditorCamera::GetPosition() const
{
    return m_Transform.GetPosition();
}
}  // namespace MatchaEditor